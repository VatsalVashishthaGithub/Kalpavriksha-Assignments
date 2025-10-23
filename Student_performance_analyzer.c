#include<stdio.h>

struct Student{
    int rollNo;
    char name[100];
    int marks[3];
    int totalMarks;
    float averageMarks;
    char grade;
};

// Here i am declaring the functions.
int calculateTotalMarks(int marks[]);
float calculateAverageMarks(int totalmarks);
char calculateGrade(float average);
void showPerformance(char grade);
void printRollNumbers(int n);

int main(){
    struct Student s[100];
    int n;      // n is the number of students
    printf("Enter the number of students: ");
    scanf("%d",&n);

    for(int i=1; i<=n; i++){
        printf("Enter details for student- %d (RollNumber , Name , Marks1 , Marks2 , Marks3) \n", i);
        scanf("%d", &s[i].rollNo);
        scanf("%s", s[i].name);
        scanf("%d", &s[i].marks[0]);
        scanf("%d", &s[i].marks[1]);
        scanf("%d", &s[i].marks[2]);

        s[i].totalMarks = calculateTotalMarks(s[i].marks);
        s[i].averageMarks = calculateAverageMarks(s[i].totalMarks);
        s[i].grade = calculateGrade(s[i].averageMarks);
    }

    // Performances of Students ::
    printf("Student Performances are following :: \n");
    for(int i=1; i<=n; i++){
        printf("Roll No. -> %d \n", s[i].rollNo);
        printf("Name -> %s \n", s[i].name);
        printf("Total Marks Scored -> %d \n", s[i].totalMarks);
        printf("Average Marks -> %f \n", s[i].averageMarks);
        printf("Grade scored -> %c \n", s[i].grade);

        if(s[i].averageMarks < 35){
            printf("Performance : Skipped (Failed)");
            continue;
        }
    
        printf("Performance : ");
        showPerformance(s[i].grade);
    }
    
    printf("\nList of Roll Numbers :: ");
    printRollNumbers(n);
    printf("\n");
    return 0;
}

/*
calculateTotalMarks():
    This function will calculate the sum total of marks obtained in all three subjects.
    It will take the array marks[] as a parameter and then calculate total marks for each student.
*/
int calculateTotalMarks(int marks[]){
    int sum = 0;
    for(int i=0; i<3; i++){
        sum += marks[i];
    }
    return sum;
}

/*
calculateAverageMarks:
    This function will calculate average from the total marks given.
    For that it will take total marks as parameter.
*/
float calculateAverageMarks(int totalMarks){
    float totalSubjects = 3.0;
    return totalMarks / totalSubjects;
}

/*
calculateGrade:
    This function with the help of average marks provided, will assign the grade for each student.
*/
char calculateGrade(float average){
    if(average >= 85 && average <= 100){
        return 'A';
    }
    else if(average >= 70){
        return 'B';
    }
    else if(average >= 50){
        return 'C';
    }
    else if(average >= 35){
        return 'D';
    }
    else{
        return 'F';
    }
}

/*
showPerformance:
    This function with the help of grades, will show the performance of each student.
    This will specifically print pattern of stars '*' for each performance.
*/
void showPerformance(char grade){
    int star = 0;
    if(grade == 'A') star = 5;
    else if(grade == 'B') star = 4;
    else if(grade == 'C') star = 3;
    else if(grade == 'D') star = 2;

    for(int i=1; i<= star; i++){
        printf("* ");
    }
    printf("\n");
}

/*
printRollNumbers:
    This function will recursively print all the Roll Numbers in ascending order.
*/
void printRollNumbers(int n){
    if(n == 0) return;
    printRollNumbers(n-1);
    printf("%d", n);
}