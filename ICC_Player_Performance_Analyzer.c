#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"  

#define MAX_TEAMS 32
#define NAME_LEN 64

typedef struct {
    char name[NAME_LEN];
    int id;
    int *idx;
    int count;
    int cap;
    float avg_bat_sr;
} TeamInfo;

static int *registry = NULL;
static int registry_count = 0;
static int registry_cap = 0;

static TeamInfo team_info[MAX_TEAMS];
static int team_info_count = 0;

static Player **extra_players = NULL;
static int extra_count = 0;
static int extra_cap = 0;

static const Player* get_player_by_registry_index(int reg_index);

static void die(const char *msg) {
    fprintf(stderr, "Fatal: %s\n", msg);
    exit(EXIT_FAILURE);
}

static void ensure_registry(int want) {
    if (registry_cap >= want) return;
    int nc = registry_cap ? registry_cap * 2 : 64;
    while (nc < want) nc *= 2;
    registry = realloc(registry, sizeof(int) * nc);
    if (!registry) die("alloc registry");
    registry_cap = nc;
}

static void ensure_extra_space(void) {
    if (extra_count + 1 <= extra_cap) return;
    int nc = extra_cap ? extra_cap * 2 : 8;
    extra_players = realloc(extra_players, sizeof(Player*) * nc);
    if (!extra_players) die("alloc extras");
    extra_cap = nc;
}

static float compute_perf(const Player *p) {
    if (!p) return 0.0f;
    if (strstr(p->role, "Batsman")) {
        return (p->battingAverage * p->strikeRate) / 100.0f;
    } else if (strstr(p->role, "Bowler")) {
        return (p->wickets * 2.0f) + (100.0f - p->economyRate);
    } else {
        return ((p->battingAverage * p->strikeRate) / 100.0f) + (p->wickets * 2.0f);
    }
}

static void team_push_idx(TeamInfo *T, int idxval) {
    if (T->count + 1 > T->cap) {
        int nc = T->cap ? T->cap * 2 : 8;
        T->idx = realloc(T->idx, sizeof(int) * nc);
        if (!T->idx) die("alloc team idx");
        T->cap = nc;
    }
    T->idx[T->count++] = idxval;
}

static void build_team_info(void) {
    char *temp_names[MAX_TEAMS];
    int tn = 0;
    for (int i = 0; i < playerCount; ++i) {
        const char *nm = players[i].team;
        int found = 0;
        for (int j = 0; j < tn; ++j) if (strcmp(temp_names[j], nm) == 0) { found = 1; break; }
        if (!found) {
            if (tn >= MAX_TEAMS) break;
            temp_names[tn++] = (char*)nm;
        }
    }
    qsort(temp_names, tn, sizeof(char*), (int(*)(const void*, const void*)) strcmp);
    team_info_count = tn;
    for (int i = 0; i < tn; ++i) {
        strncpy(team_info[i].name, temp_names[i], NAME_LEN - 1);
        team_info[i].name[NAME_LEN - 1] = '\0';
        team_info[i].id = i + 1;
        team_info[i].idx = NULL;
        team_info[i].count = 0;
        team_info[i].cap = 0;
        team_info[i].avg_bat_sr = 0.0f;
    }
}

static int find_team_idx_by_name(const char *name) {
    for (int i = 0; i < team_info_count; ++i) if (strcmp(team_info[i].name, name) == 0) return i;
    return -1;
}

static void index_players(void) {
    registry_count = 0;
    for (int i = 0; i < team_info_count; ++i) {
        free(team_info[i].idx);
        team_info[i].idx = NULL;
        team_info[i].count = 0;
        team_info[i].cap = 0;
        team_info[i].avg_bat_sr = 0.0f;
    }
    ensure_registry(playerCount + 1);
    for (int i = 0; i < playerCount; ++i) {
        registry[registry_count++] = i;
        int t = find_team_idx_by_name(players[i].team);
        if (t >= 0) team_push_idx(&team_info[t], registry_count - 1);
    }
    for (int e = 0; e < extra_count; ++e) {
        registry[registry_count++] = playerCount + e; 
        int t = find_team_idx_by_name(extra_players[e]->team);
        if (t < 0) {
            if (team_info_count + 1 > MAX_TEAMS) continue;
            int pos = team_info_count;
            for (int j = 0; j < team_info_count; ++j) {
                if (strcmp(extra_players[e]->team, team_info[j].name) < 0) { pos = j; break; }
            }
            for (int j = team_info_count; j > pos; --j) team_info[j] = team_info[j-1];
            strncpy(team_info[pos].name, extra_players[e]->team, NAME_LEN - 1);
            team_info[pos].name[NAME_LEN-1] = '\0';
            team_info[pos].id = pos + 1;
            team_info[pos].idx = NULL;
            team_info[pos].count = 0;
            team_info[pos].cap = 0;
            team_info[pos].avg_bat_sr = 0.0f;
            team_info_count++;
            for (int k = 0; k < team_info_count; ++k) team_info[k].id = k + 1;
            t = pos;
        }
        team_push_idx(&team_info[t], registry_count - 1);
    }
    for (int t = 0; t < team_info_count; ++t) {
        float sum = 0.0f; int cnt = 0;
        for (int j = 0; j < team_info[t].count; ++j) {
            const Player *pp = get_player_by_registry_index(team_info[t].idx[j]);
            if (!pp) continue;
            if (strstr(pp->role, "Batsman") || strstr(pp->role, "All")) {
                sum += pp->strikeRate;
                cnt++;
            }
        }
        team_info[t].avg_bat_sr = cnt ? (sum / cnt) : 0.0f;
    }
}

// It is a comparator function for sorting teams by avg SR descending
static int cmp_team_avg_desc(const void *a, const void *b) {
    TeamInfo *A = *(TeamInfo**)a;
    TeamInfo *B = *(TeamInfo**)b;
    if (A->avg_bat_sr < B->avg_bat_sr) return 1;
    if (A->avg_bat_sr > B->avg_bat_sr) return -1;
    return 0;
}

// It is a comparator function for registry indices by performance (desc) 
static int cmp_registry_perf_desc(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    const Player *pa = get_player_by_registry_index(ia);
    const Player *pb = get_player_by_registry_index(ib);
    float fa = compute_perf(pa);
    float fb = compute_perf(pb);
    if (fa < fb) return 1;
    if (fa > fb) return -1;
    return 0;
}

// It is a helper function that returns Player* for a registry index (0..registry_count-1)
static const Player* get_player_by_registry_index(int reg_index) {
    if (reg_index < 0 || reg_index >= registry_count) return NULL;
    int raw = registry[reg_index];
    if (raw < playerCount) return &players[raw];
    int ex = raw - playerCount;
    if (ex < extra_count) return extra_players[ex];
    return NULL;
}

// this function prints header for player table
static void print_player_table_header(void) {
    printf("%-5s %-22s %-14s %-6s %-6s %-6s %-6s\n",
           "ID","Name","Role","Runs","Avg","SR","Wkts");
}

static void action_show_team(void) {
    printf("Enter Team ID (1..%d): ", team_info_count);
    int id; if (scanf("%d", &id) != 1) { while(getchar()!='\n'); return; }
    if (id < 1 || id > team_info_count) { puts("Invalid team id"); return; }
    TeamInfo *T = &team_info[id - 1];
    printf("\nTeam: %s (ID %d)  Players: %d  AvgBatSR: %.2f\n",
           T->name, T->id, T->count, T->avg_bat_sr);
    print_player_table_header();
    for (int i = 0; i < T->count; ++i) {
        const Player *pp = get_player_by_registry_index(T->idx[i]);
        if (!pp) continue;
        printf("%-5d %-22s %-14s %-6d %-6.2f %-6.2f %-6d\n",
               pp->id, pp->name, pp->role, pp->totalRuns,
               pp->battingAverage, pp->strikeRate, pp->wickets);
    }
}

static void action_show_teams_sorted(void) {
    TeamInfo *arr[MAX_TEAMS];
    int n = team_info_count;
    for (int i = 0; i < n; ++i) arr[i] = &team_info[i];
    qsort(arr, n, sizeof(TeamInfo*), cmp_team_avg_desc);
    printf("\nTeams by Avg Batting SR:\n");
    printf("%-4s %-22s %-8s %-8s\n", "ID","Team","AvgSR","Players");
    for (int i = 0; i < n; ++i) {
        printf("%-4d %-22s %-8.2f %-8d\n", arr[i]->id, arr[i]->name, arr[i]->avg_bat_sr, arr[i]->count);
    }
}

// this function show top K by role inside a team 
static void action_top_k_role(void) {
    printf("Team ID (1..%d): ", team_info_count);
    int id; if (scanf("%d", &id) != 1) { while(getchar()!='\n'); return; }
    if (id < 1 || id > team_info_count) { puts("Invalid team id"); return; }
    char rolebuf[32];
    printf("Role (Batsman/Bowler/All-rounder): "); scanf(" %[^\n]s", rolebuf);
    printf("K: "); int K; if (scanf("%d", &K) != 1) { while(getchar()!='\n'); return; }
    TeamInfo *T = &team_info[id - 1];
    int *temp = malloc(sizeof(int) * T->count);
    int tc = 0;
    for (int i = 0; i < T->count; ++i) {
        const Player *pp = get_player_by_registry_index(T->idx[i]);
        if (!pp) continue;
        if ((strstr(rolebuf, "Batsman") && strstr(pp->role, "Batsman")) ||
            (strstr(rolebuf, "Bowler")  && strstr(pp->role, "Bowler")) ||
            (strstr(rolebuf, "All")     && !strstr(pp->role, "Batsman") && !strstr(pp->role, "Bowler"))) {
            temp[tc++] = T->idx[i];
        }
    }
    if (tc == 0) { puts("No players found for that role in this team"); free(temp); return; }
    qsort(temp, tc, sizeof(int), cmp_registry_perf_desc);
    if (K > tc) K = tc;
    print_player_table_header();
    for (int i = 0; i < K; ++i) {
        const Player *pp = get_player_by_registry_index(temp[i]);
        if (!pp) continue;
        printf("%-5d %-22s %-14s %-6d %-6.2f %-6.2f %-6d\n",
               pp->id, pp->name, pp->role, pp->totalRuns, pp->battingAverage, pp->strikeRate, pp->wickets);
    }
    free(temp);
}

// This function will show all players of a role across teams
static void action_show_all_role(void) {
    char rolebuf[32];
    printf("Role (Batsman/Bowler/All-rounder): "); scanf(" %[^\n]s", rolebuf);
    int *temp = malloc(sizeof(int) * registry_count);
    int tc = 0;
    for (int i = 0; i < registry_count; ++i) {
        const Player *pp = get_player_by_registry_index(i);
        if (!pp) continue;
        if ((strstr(rolebuf, "Batsman") && strstr(pp->role, "Batsman")) ||
            (strstr(rolebuf, "Bowler")  && strstr(pp->role, "Bowler")) ||
            (strstr(rolebuf, "All")     && !strstr(pp->role, "Batsman") && !strstr(pp->role, "Bowler"))) {
            temp[tc++] = i;
        }
    }
    if (tc == 0) { puts("No players found"); free(temp); return; }
    qsort(temp, tc, sizeof(int), cmp_registry_perf_desc);
    printf("\nAll players (%s):\n", rolebuf);
    printf("%-5s %-22s %-14s %-6s %-6s %-6s %-20s\n", "ID","Name","Role","Runs","Avg","SR","Team");
    for (int i = 0; i < tc; ++i) {
        const Player *pp = get_player_by_registry_index(temp[i]);
        if (!pp) continue;
        printf("%-5d %-22s %-14s %-6d %-6.2f %-6.2f %-20s\n",
               pp->id, pp->name, pp->role, pp->totalRuns, pp->battingAverage, pp->strikeRate, pp->team);
    }
    free(temp);
}

//  This function adds a new player at runtime (heap-alloc Player), then reindex 
static void action_add_player(void) {
    Player *np = malloc(sizeof(Player));
    if (!np) die("alloc new player");
    printf("Enter id: "); if (scanf("%d", &np->id) != 1) { free(np); while(getchar()!='\n'); return; }
    printf("Enter name: "); scanf(" %[^\n]s", np->name);
    printf("Enter team (exact): "); scanf(" %[^\n]s", np->team);
    printf("Enter role (Batsman/Bowler/All-rounder): "); scanf(" %[^\n]s", np->role);
    printf("totalRuns battingAverage strikeRate wickets economyRate: ");
    if (scanf("%d %f %f %d %f", &np->totalRuns, &np->battingAverage, &np->strikeRate, &np->wickets, &np->economyRate) != 5) {
        free(np); while(getchar()!='\n'); return;
    }
    ensure_extra_space();
    extra_players[extra_count++] = np;
    index_players();
    printf("Player added.\n");
}

static void cleanup(void) {
    free(registry);
    for (int i = 0; i < team_info_count; ++i){
        free(team_info[i].idx); 
    }
    for (int i = 0; i < extra_count; ++i){
        free(extra_players[i]);
    } 
    free(extra_players);
}

int main(void) {
    build_team_info();
    index_players();

    while(1) {
        printf("\nMenu:\n");
        printf("1 Show players of a team\n");
        printf("2 Show teams sorted by avg batting SR\n");
        printf("3 Top K players of a team by role\n");
        printf("4 Show all players by role\n");
        printf("5 Add player (runtime)\n");
        printf("6 Exit\n");
        printf("Choice: ");
        int ch;
        if (scanf("%d", &ch) != 1) { while(getchar()!='\n'); continue; }
        switch (ch) {
            case 1: action_show_team(); break;
            case 2: action_show_teams_sorted(); break;
            case 3: action_top_k_role(); break;
            case 4: action_show_all_role(); break;
            case 5: action_add_player(); break;
            case 6: cleanup(); printf("Bye.\n"); return 0;
            default: puts("Invalid option"); break;
        }
    }
    return 0;
}
