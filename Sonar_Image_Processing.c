#include<stdio.h>
#include<stdlib.h>
#include<time.h>

void printMatrix(int,int*);
void rotateMatrix(int,int*);
void applySmoothingFilter(int,int*);
int neighbourTraverse(int,int,int,int*);

/*
printMatrix:
    This function will just print the matrix.
*/
void printMatrix(int n, int *matrix){
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            printf("%5d", *(matrix + i*n + j));
        }
        printf("\n");
    }
}

/*
rotateMatrix:
    This function will rotate the matrix 90 degrees clockwise each time called.
*/
void rotateMatrix(int n, int *matrix){
    for(int i=0; i<n/2; i++){
        int start = i;
        int end = n - i - 1;
        for(int j=start; j<end; j++){
            int offset = j - start;
            // these will be the four corner points
            int *top = (matrix + start*n + j);
            int *left = (matrix + (end-offset)*n + start);
            int *bottom = (matrix + end*n + (end-offset));
            int *right = (matrix + j*n + end);
            // performing cyclic swap of the values
            int temp = *top;
            *top = *left;
            *left = *bottom;
            *bottom = *right;
            *right = temp;
        }
    }
}

/*
applySmoothingFilter:
    This function will take the average of the nearby values and replace the current value with average value.
*/
void applySmoothingFilter(int n, int * matrix){
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            *(matrix + i*n + j) = neighbourTraverse(i,j,n,matrix);
        }
    }
}

/*
neighbourTraverse:
    This function will actually traverse through the neighbour values while considering the edge cases.
*/
int neighbourTraverse(int row, int col, int n, int *matrix){
    int sum = 0;
    int count = 0;
    for(int drow=-1; drow<=1; drow++){
        for(int dcol=-1; dcol<=1; dcol++){
            int nrow = row + drow;
            int ncol = col + dcol;
            if(nrow >= 0 && nrow < n && ncol >= 0 && ncol < n){
                sum += *(matrix + nrow*n + ncol);
                count += 1;
            }
        }
    }
    return sum/count;
}

int main(){
    int n;
    printf("Enter size of matrix (2-10) : ");
    scanf("%d",&n);
    if(n < 2 || n > 10){
        printf("Error!.. Enter valid size");
        return 0;
    }
    int *matrix = (int *) malloc(n*n * sizeof(int));

    srand(time(0));
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            *(matrix + i*n + j) = rand() % 256;
        }
    }

    printf("\nOriginal Matrix..\n");
    printMatrix(n,matrix);

    printf("\nAfter rotating 90-degree clockwise..\n");
    rotateMatrix(n,matrix);
    printMatrix(n,matrix);

    printf("After applying 3*3 smoothing filter..\n");
    applySmoothingFilter(n,matrix);
    printMatrix(n,matrix);
    
    free(matrix);
    return 0;
}