#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NAME_SIZE 100

struct Product {
    int productID;
    char productName[NAME_SIZE];
    float price;
    int quantity;
};

void selectChoice(struct Product *, int, int);
void addNewProduct(struct Product **, int *);
void viewAllProducts(struct Product *, int );
void updateQuantity(struct Product *, int );
void searchByID(struct Product *, int );
void searchByName(struct Product *, int );
void searchByPriceRange(struct Product *, int );
void deleteByID(struct Product **, int *);

int main() {
    int totalNumberOfProducts;
    printf("Enter total number of products you want to add initially: ");
    scanf("%d", &totalNumberOfProducts);

    struct Product *myProducts = (struct Product *) calloc(totalNumberOfProducts, sizeof(struct Product));

    for (int i = 1; i <= totalNumberOfProducts; i++) {
        printf("\nEnter details for product %d ::\n", i);
        printf("Enter Product Name: ");
        scanf("%s", myProducts[i].productName);
        printf("Enter Product ID: ");
        scanf("%d", &myProducts[i].productID);
        printf("Enter Product Price: ");
        scanf("%f", &myProducts[i].price);
        printf("Enter Product Quantity: ");
        scanf("%d", &myProducts[i].quantity);
    }

    int choice;
    while (1) {
        printf("\n===== MENU =====\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by ID\n");
        printf("5. Search Products by Name\n");
        printf("6. Search Products by Price Range\n");
        printf("7. Delete Product by ID\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        selectChoice(myProducts,totalNumberOfProducts,choice);
    }
    return 0;
}

void selectChoice(struct Product *myProducts, int totalNumberOfProducts, int choice){
    switch (choice) {
            case 1: addNewProduct(&myProducts, &totalNumberOfProducts); break;
            case 2: viewAllProducts(myProducts, totalNumberOfProducts); break;
            case 3: updateQuantity(myProducts, totalNumberOfProducts); break;
            case 4: searchByID(myProducts, totalNumberOfProducts); break;
            case 5: searchByName(myProducts, totalNumberOfProducts); break;
            case 6: searchByPriceRange(myProducts, totalNumberOfProducts); break;
            case 7: deleteByID(&myProducts, &totalNumberOfProducts); break;
            case 8:
                free(myProducts);
                printf("Memory freed. Exiting program.\n");
                exit(0);
            default:
                printf("Your choice is Invalid! Try again.\n");
        }
}

void addNewProduct(struct Product **products, int *count) {
    *products = (struct Product *) realloc(*products, (*count + 1) * sizeof(struct Product));

    printf("\nEnter details for new product:\n");
    printf("Enter Product Name: ");
    scanf("%s", (*products)[*count].productName);
    printf("Enter Product ID: ");
    scanf("%d", &(*products)[*count].productID);
    printf("Enter Product Price: ");
    scanf("%f", &(*products)[*count].price);
    printf("Enter Product Quantity: ");
    scanf("%d", &(*products)[*count].quantity);

    (*count)++;
    printf("Product added successfully!\n");
}

void viewAllProducts(struct Product *products, int count) {
    if (count == 0) {
        printf("No products available.\n");
        return;
    }
    printf("\n=== Product List ===\n");
    for (int i = 0; i < count; i++) {
        printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
               products[i].productID, products[i].productName,
               products[i].price, products[i].quantity);
    }
}

void updateQuantity(struct Product *products, int count) {
    int id, newQty;
    printf("Enter Product ID to update: ");
    scanf("%d", &id);
    for (int i = 0; i < count; i++) {
        if (products[i].productID == id) {
            printf("Enter new quantity: ");
            scanf("%d", &newQty);
            products[i].quantity = newQty;
            printf("Quantity updated!\n");
            return;
        }
    }
    printf("Product not found!\n");
}

void searchByID(struct Product *products, int count) {
    int id;
    printf("Enter Product ID to search: ");
    scanf("%d", &id);
    for (int i = 0; i < count; i++) {
        if (products[i].productID == id) {
            printf("Found: %s | Price: %.2f | Quantity: %d\n",
                   products[i].productName, products[i].price, products[i].quantity);
            return;
        }
    }
    printf("Product not found!\n");
}

void searchByName(struct Product *products, int count) {
    char name[NAME_SIZE];
    printf("Enter product name (partial allowed): ");
    scanf("%s", name);
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strstr(products[i].productName, name)) {
            printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   products[i].productID, products[i].productName,
                   products[i].price, products[i].quantity);
            found = 1;
        }
    }
    if (!found) printf("No products match the name.\n");
}

void searchByPriceRange(struct Product *products, int count) {
    float low, high;
    printf("Enter minimum and maximum price: ");
    scanf("%f %f", &low, &high);
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (products[i].price >= low && products[i].price <= high) {
            printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
                   products[i].productID, products[i].productName,
                   products[i].price, products[i].quantity);
            found = 1;
        }
    }
    if (!found) printf("No products in that price range.\n");
}

void deleteByID(struct Product **products, int *count) {
    int id;
    printf("Enter Product ID to delete: ");
    scanf("%d", &id);
    int index = -1;
    for (int i = 0; i < *count; i++) {
        if ((*products)[i].productID == id) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("Product not found!\n");
        return;
    }

    for (int i = index; i < *count - 1; i++) {
        (*products)[i] = (*products)[i + 1];
    }

    *products = (struct Product *) realloc(*products, (*count - 1) * sizeof(struct Product));
    (*count)--;
    printf("Product deleted successfully!\n");
}

