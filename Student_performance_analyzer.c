#include<stdio.h>
#define MAX_NAME_SIZE 100
#define totalSubjects 3

enum Grade{
    Grade_A,
    Grade_B,
    Grade_C,
    Grade_D,
    Grade_F
};
struct Student{
    int rollNo;
    char name[MAX_NAME_SIZE];
    int marks[totalSubjects];
    int totalMarks;
    float averageMarks;
    enum Grade grade;
};

int getTotalStudents();
void inputStudentData(struct Student *s, int studentIndex);
int calculateTotalMarks(int marks[]);
float calculateAverageMarks(int totalmarks);
enum Grade calculateGrade(float average);
void showPerformance(enum Grade grade);
void printRollNumbers(int totalStudent);

int main(){
    struct Student s[100];
    int totalStudent = getTotalStudents();
    for(int i=1; i<=totalStudent; i++){
        inputStudentData(&s[i], i);
    }

    printf("Student Performances are following :: \n");
    for(int i=1; i<=totalStudent; i++){
        printf("Roll No. -> %d \n", s[i].rollNo);
        printf("Name -> %s \n", s[i].name);
        printf("Total Marks Scored -> %d \n", s[i].totalMarks);
        printf("Average Marks -> %f \n", s[i].averageMarks);

        char gradeChar;
        switch(s[i].grade){
            case Grade_A : gradeChar = 'A'; break;
            case Grade_B : gradeChar = 'B'; break;
            case Grade_C : gradeChar = 'C'; break;
            case Grade_D : gradeChar = 'D'; break;
            case Grade_F : gradeChar = 'F'; break;
        }
        printf("Grade scored -> %c \n", gradeChar);

        if(s[i].averageMarks < 35){
            printf("Performance : Skipped (Failed)");
            continue;
        }
    
        printf("Performance : ");
        showPerformance(s[i].grade);
    }
    
    printf("\nList of Roll Numbers :: ");
    printRollNumbers(totalStudent);
    printf("\n");
    return 0;
}

/*
getTotalStudents():
    This function will input the total number of students.
*/
int  getTotalStudents(){
    int total;      
    printf("Enter total number of students: ");
    scanf("%d",&total);
    return total;
}

/*
inputStudentData():
    This function will input the data of all students.
*/
void inputStudentData(struct Student *s, int studentIndex){
    printf("Enter details for student-%d (RollNumber , Name , Marks1 , Marks2 , Marks3)\n", studentIndex);
    scanf("%d", &s->rollNo);
    scanf("%s", s->name);
    scanf("%d", &s->marks[0]);
    scanf("%d", &s->marks[1]);
    scanf("%d", &s->marks[2]);

    s->totalMarks = calculateTotalMarks(s->marks);
    s->averageMarks = calculateAverageMarks(s->totalMarks);
    s->grade = calculateGrade(s->averageMarks);
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
*/
float calculateAverageMarks(int totalMarks){
    return totalMarks / totalSubjects;
}

/*
calculateGrade:
    This function will assign the grade for each student.
*/
enum Grade calculateGrade(float average){
    if(average >= 85 && average <= 100){
        return Grade_A;
    }
    else if(average >= 70){
        return Grade_B;
    }
    else if(average >= 50){
        return Grade_C;
    }
    else if(average >= 35){
        return Grade_D;
    }
    else{
        return Grade_F;
    }
}

/*
showPerformance:
    This function will show the performance of each student.
    This will specifically print pattern of stars '*' for each performance.
*/
void showPerformance(enum Grade grade){
    int star = 0;
    if(grade == Grade_A) star = 5;
    else if(grade == Grade_B) star = 4;
    else if(grade == Grade_C) star = 3;
    else if(grade == Grade_D) star = 2;

    for(int i=1; i<= star; i++){
        printf("* ");
    }
    printf("\n");
}

/*
printRollNumbers:
    This function will recursively print all the Roll Numbers in ascending order.
*/
void printRollNumbers(int totalStudent){
    if(totalStudent == 0) return;
    printRollNumbers(totalStudent-1);
    printf("%d", totalStudent);
}