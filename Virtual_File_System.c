#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024   
#define MAX_NAME 50
#define MAX_BLOCKS_PER_FILE 5000  

static unsigned char virtualDisk[NUM_BLOCKS][BLOCK_SIZE];

typedef struct FreeBlock {
    int index;
    struct FreeBlock *prev, *next;
} FreeBlock;

typedef struct {
    FreeBlock *head;
    FreeBlock *tail;
    int freeCount;
} FreeList;

typedef struct FileNode {
    char name[MAX_NAME+1];
    int isDir;
    struct FileNode *parent;
    struct FileNode *next;
    struct FileNode *prev;
    struct FileNode *child; 
    int blockPointers[MAX_BLOCKS_PER_FILE];
    int blocksUsed;
    int contentSize;
} FileNode;

static FreeList freeList = {NULL, NULL, 0};
static FileNode *root = NULL;
static FileNode *cwd = NULL;

void init_free_list() {
    freeList.head = freeList.tail = NULL;
    freeList.freeCount = 0;
    for (int i = 0; i < NUM_BLOCKS; ++i) {
        FreeBlock *fb = (FreeBlock*)malloc(sizeof(FreeBlock));
        fb->index = i;
        fb->prev = freeList.tail;
        fb->next = NULL;
        if (freeList.tail) freeList.tail->next = fb;
        freeList.tail = fb;
        if (!freeList.head) freeList.head = fb;
        freeList.freeCount++;
    }
}

int allocate_block_from_head() {
    if (!freeList.head) return -1;
    FreeBlock *fb = freeList.head;
    int idx = fb->index;
    freeList.head = fb->next;
    if (freeList.head) freeList.head->prev = NULL;
    else freeList.tail = NULL;
    freeList.freeCount--;
    free(fb);
    return idx;
}

void free_block_to_tail(int idx) {
    FreeBlock *fb = (FreeBlock*)malloc(sizeof(FreeBlock));
    fb->index = idx;
    fb->next = NULL;
    fb->prev = freeList.tail;
    if (freeList.tail) freeList.tail->next = fb;
    freeList.tail = fb;
    if (!freeList.head) freeList.head = fb;
    freeList.freeCount++;
}

/* Filesystem helpers */
FileNode* make_node(const char *name, int isDir) {
    FileNode *n = (FileNode*)malloc(sizeof(FileNode));
    strncpy(n->name, name, MAX_NAME);
    n->name[MAX_NAME] = '\0';
    n->isDir = isDir;
    n->parent = NULL;
    n->next = n->prev = NULL;
    n->child = NULL;
    n->blocksUsed = 0;
    n->contentSize = 0;
    for (int i = 0; i < MAX_BLOCKS_PER_FILE; ++i) n->blockPointers[i] = -1;
    return n;
}

/* Insert child into parent's circular list (at end) */
void insert_child(FileNode *parent, FileNode *child) {
    child->parent = parent;
    if (!parent->child) {
        parent->child = child;
        child->next = child->prev = child;
    } else {
        FileNode *first = parent->child;
        FileNode *last = first->prev;
        last->next = child;
        child->prev = last;
        child->next = first;
        first->prev = child;
    }
}

/* Remove node from sibling circular list */
void remove_from_parent(FileNode *node) {
    FileNode *p = node->parent;
    if (!p) return;
    if (p->child == node) {
        if (node->next == node) {
            p->child = NULL;
        } else {
            p->child = node->next;
        }
    }
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = node->prev = NULL;
    node->parent = NULL;
}

/* Find child by name in current directory. Returns pointer or NULL */
FileNode* find_in_cwd(const char *name) {
    FileNode *c = cwd->child;
    if (!c) return NULL;
    FileNode *it = c;
    do {
        if (strcmp(it->name, name) == 0) return it;
        it = it->next;
    } while (it != c);
    return NULL;
}

void cmd_mkdir(char *arg) {
    if (!arg) { printf("Usage: mkdir <dirname>\n"); return; }
    if (strlen(arg) > MAX_NAME) { printf("Name too long\n"); return; }
    if (find_in_cwd(arg)) { printf("Entry already exists\n"); return; }
    FileNode *dir = make_node(arg, 1);
    insert_child(cwd, dir);
    printf("Directory '%s' created\n", arg);
}

void cmd_create(char *arg) {
    if (!arg) { printf("Usage: create <filename>\n"); return; }
    if (strlen(arg) > MAX_NAME) { printf("Name too long\n"); return; }
    if (find_in_cwd(arg)) { printf("Entry already exists\n"); return; }
    FileNode *f = make_node(arg, 0);
    insert_child(cwd, f);
    printf("File '%s' created\n", arg);
}

int blocks_needed(int bytes) {
    if (bytes <= 0) return 0;
    return (bytes + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

void cmd_write(char *arg) {
    if (!arg) { printf("Usage: write <filename>\n"); return; }
    FileNode *f = find_in_cwd(arg);
    if (!f) { printf("File not found\n"); return; }
    if (f->isDir) { printf("Is a directory\n"); return; }

    printf("Enter content (end with a single line containing only ~):\n");
    char buffer[4096];
    unsigned char *content = NULL;
    size_t total = 0;
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strcmp(buffer, "~\n") == 0 || strcmp(buffer, "~\r\n") == 0) break;
        size_t len = strlen(buffer);
        unsigned char *tmp = realloc(content, total + len);
        if (!tmp) { free(content); printf("Memory error\n"); return; }
        content = tmp;
        memcpy(content + total, buffer, len);
        total += len;
    }
    int need = blocks_needed((int)total);
    if (need > freeList.freeCount) {
        printf("Not enough disk space. Needed %d blocks, free %d\n", need, freeList.freeCount);
        free(content);
        return;
    }

    for (int i = 0; i < f->blocksUsed; ++i) {
        if (f->blockPointers[i] >= 0) free_block_to_tail(f->blockPointers[i]);
        f->blockPointers[i] = -1;
    }
    f->blocksUsed = 0;
    f->contentSize = 0;

    size_t written = 0;
    for (int b = 0; b < need; ++b) {
        int idx = allocate_block_from_head();
        if (idx < 0) { printf("Unexpected allocation failure\n"); break; }
        f->blockPointers[b] = idx;
        f->blocksUsed++;
        int tocopy = (int) ((total - written) < BLOCK_SIZE ? (total - written) : BLOCK_SIZE);
        if (tocopy > 0) memcpy(virtualDisk[idx], content + written, tocopy);
        if (tocopy < BLOCK_SIZE) virtualDisk[idx][tocopy] = '\0';
        written += tocopy;
    }
    f->contentSize = (int)total;
    printf("Wrote %zu bytes into %d blocks\n", written, f->blocksUsed);
    free(content);
}

void cmd_read(char *arg) {
    if (!arg) { printf("Usage: read <filename>\n"); return; }
    FileNode *f = find_in_cwd(arg);
    if (!f) { printf("File not found\n"); return; }
    if (f->isDir) { printf("Is a directory\n"); return; }
    if (f->blocksUsed == 0) { printf("File is empty\n"); return; }

    int remaining = f->contentSize;
    for (int i = 0; i < f->blocksUsed && remaining > 0; ++i) {
        int idx = f->blockPointers[i];
        int take = remaining < BLOCK_SIZE ? remaining : BLOCK_SIZE;
        fwrite(virtualDisk[idx], 1, take, stdout);
        remaining -= take;
    }
    printf("\n");
}

void cmd_delete(char *arg) {
    if (!arg) { printf("Usage: delete <filename>\n"); return; }
    FileNode *f = find_in_cwd(arg);
    if (!f) { printf("File not found\n"); return; }
    if (f->isDir) { printf("Is a directory\n"); return; }

    for (int i = 0; i < f->blocksUsed; ++i) {
        if (f->blockPointers[i] >= 0) free_block_to_tail(f->blockPointers[i]);
    }
    remove_from_parent(f);
    free(f);
    printf("File deleted\n");
}

void cmd_rmdir(char *arg) {
    if (!arg) { printf("Usage: rmdir <dirname>\n"); return; }
    FileNode *d = find_in_cwd(arg);
    if (!d) { printf("Directory not found\n"); return; }
    if (!d->isDir) { printf("Not a directory\n"); return; }
    if (d->child) { printf("Directory not empty\n"); return; }
    remove_from_parent(d);
    free(d);
    printf("Directory removed\n");
}

void cmd_ls() {
    FileNode *c = cwd->child;
    if (!c) { printf("(empty)\n"); return; }
    FileNode *it = c;
    do {
        if (it->isDir) printf("[DIR] %s\n", it->name);
        else printf("[FILE] %s (%d bytes, %d blocks)\n", it->name, it->contentSize, it->blocksUsed);
        it = it->next;
    } while (it != c);
}

void cmd_cd(char *arg) {
    if (!arg) { printf("Usage: cd <dirname>\n"); return; }
    if (strcmp(arg, "/") == 0) { cwd = root; return; }
    if (strcmp(arg, "..") == 0) {
        if (cwd->parent) cwd = cwd->parent;
        return;
    }
    FileNode *d = find_in_cwd(arg);
    if (!d) { printf("Not found\n"); return; }
    if (!d->isDir) { printf("Not a directory\n"); return; }
    cwd = d;
}

void cmd_pwd() {
    char stack[100][MAX_NAME+1];
    int sp = 0;
    FileNode *it = cwd;
    while (it && it != root) {
        strncpy(stack[sp++], it->name, MAX_NAME);
        stack[sp-1][MAX_NAME] = '\0';
        it = it->parent;
    }
    printf("/");
    for (int i = sp-1; i >= 0; --i) {
        printf("%s/", stack[i]);
    }
    printf("\n");
}

void cmd_df() {
    int used = NUM_BLOCKS - freeList.freeCount;
    printf("Total blocks: %d\n", NUM_BLOCKS);
    printf("Used blocks: %d\n", used);
    printf("Free blocks: %d\n", freeList.freeCount);
}

/* cleanup all nodes recursively */
void free_tree(FileNode *node) {
    if (!node) return;
    FileNode *c = node->child;
    if (c) {
        FileNode *it = c->next;
        while (it != c) {
            FileNode *next = it->next;
            free_tree(it);
            it = next;
        }
        free_tree(c);
    }
    if (!node->isDir) {
        for (int i = 0; i < node->blocksUsed; ++i) if (node->blockPointers[i] >= 0) free_block_to_tail(node->blockPointers[i]);
    }
    free(node);
}

void cleanup() {
    if (root) {
        FileNode *c = root->child;
        if (c) {
            FileNode *it = c->next;
            while (it != c) {
                FileNode *next = it->next;
                free_tree(it);
                it = next;
            }
            free_tree(c);
        }
        free(root);
        root = cwd = NULL;
    }
    FreeBlock *fb = freeList.head;
    while (fb) {
        FreeBlock *nx = fb->next;
        free(fb);
        fb = nx;
    }
    freeList.head = freeList.tail = NULL;
    freeList.freeCount = 0;
}

char *next_token(char **line) {
    char *s = *line;
    if (!s) return NULL;
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) { *line = NULL; return NULL; }
    char *start = s;
    while (*s && !isspace((unsigned char)*s)) s++;
    if (*s) { *s = '\0'; s++; }
    *line = s;
    return start;
}

int main() {
    init_free_list();
    root = make_node("", 1);
    root->parent = NULL;
    root->child = NULL;
    root->next = root->prev = root;
    cwd = root;

    printf("Simple VFS initialized with %d blocks (block size %d bytes)\n", NUM_BLOCKS, BLOCK_SIZE);
    printf("Type 'help' for list of commands\n");

    char cmdline[1024];
    while (1) {
        printf("vfs> ");
        if (!fgets(cmdline, sizeof(cmdline), stdin)) break;
        size_t L = strlen(cmdline);
        if (L && (cmdline[L-1] == '\n' || cmdline[L-1] == '\r')) cmdline[L-1] = '\0';
        char *p = cmdline;
        char *t = next_token(&p);
        if (!t) continue;
        if (strcmp(t, "help") == 0) {
            printf("Commands: mkdir, create, write, read, delete, rmdir, ls, cd, pwd, df, exit\n");
        } else if (strcmp(t, "mkdir") == 0) {
            char *a = next_token(&p);
            cmd_mkdir(a);
        } else if (strcmp(t, "create") == 0) {
            char *a = next_token(&p);
            cmd_create(a);
        } else if (strcmp(t, "write") == 0) {
            char *a = next_token(&p);
            cmd_write(a);
        } else if (strcmp(t, "read") == 0) {
            char *a = next_token(&p);
            cmd_read(a);
        } else if (strcmp(t, "delete") == 0) {
            char *a = next_token(&p);
            cmd_delete(a);
        } else if (strcmp(t, "rmdir") == 0) {
            char *a = next_token(&p);
            cmd_rmdir(a);
        } else if (strcmp(t, "ls") == 0) {
            cmd_ls();
        } else if (strcmp(t, "cd") == 0) {
            char *a = next_token(&p);
            cmd_cd(a);
        } else if (strcmp(t, "pwd") == 0) {
            cmd_pwd();
        } else if (strcmp(t, "df") == 0) {
            cmd_df();
        } else if (strcmp(t, "exit") == 0) {
            break;
        } else {
            printf("Unknown command: %s\n", t);
        }
    }
    cleanup();
    printf("Exiting. Freed memory.\n");
    return 0;
}
