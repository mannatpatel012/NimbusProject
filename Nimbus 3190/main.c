#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COURSES 10
#define MAX_CODE_LEN 10
#define MAX_NAME_LEN 50
#define MAX_PREREQ 5
#define CREDIT_OVERLOAD_LIMIT 18

// Structure to represent a Course
typedef struct {
    char code[MAX_CODE_LEN];
    char name[MAX_NAME_LEN];
    int credits;
    int semesterOffered;
    int timeSlot; // e.g., 1 = Morning Slot, 2 = Noon Slot, 3 = Evening Slot
    char prerequisites[MAX_PREREQ][MAX_CODE_LEN];
    int prereqCount;
} Course;

// Structure to represent a Student's Academic Record & Plan
typedef struct {
    char studentName[MAX_NAME_LEN];
    
    // Completed courses history
    char completedCourses[MAX_COURSES][MAX_CODE_LEN];
    int completedCount;
    
    // Current semester plan ( dynamically managed )
    int *plannedCourseIndices;
    int plannedCount;
    int plannedCapacity;
} Student;

// Global Course Catalog
Course catalog[MAX_COURSES];
int totalCourses = 0;

// Function Prototypes
void initializeCatalog();
void displayCatalog();
int findCourseIndex(const char *code);
bool isCourseCompleted(Student *s, const char *code);
bool checkPrerequisitesRecursive(int courseIndex, Student *s, bool visited[], bool stack[]);
bool hasCyclicPrerequisites();
void addCourseToPlan(Student *s);
void viewStudentPlan(Student *s);
void freeStudent(Student *s);

int main() {
    initializeCatalog();
    
    Student student;
    printf("Enter Student Name: ");
    scanf(" %[^\n]", student.studentName);
    
    student.completedCount = 0;
    student.plannedCount = 0;
    student.plannedCapacity = 2;
    student.plannedCourseIndices = (int *)malloc(student.plannedCapacity * sizeof(int));
    
    // Sample Completed Courses setup (for testing prerequisite validation)
    printf("Enter number of previously completed courses: ");
    int n;
    scanf("%d", &n);
    printf("Enter course codes of completed courses (space-separated or one by one):\n");
    for (int i = 0; i < n && i < MAX_COURSES; i++) {
        scanf("%s", student.completedCourses[i]);
        student.completedCount++;
    }

    int choice;
    do {
        printf("\n=========================================\n");
        printf("     STUDENT COURSE PLANNER & CHECKER    \n");
        printf("=========================================\n");
        printf("1. View Course Catalog\n");
        printf("2. Check Prerequisite Graph for Cycles\n");
        printf("3. Add Course to Semester Plan\n");
        printf("4. View Current Semester Plan & Totals\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                displayCatalog();
                break;
            case 2:
                if (hasCyclicPrerequisites()) {
                    printf("\n[WARNING] Cyclic dependency detected in the course prerequisites catalog!\n");
                } else {
                    printf("\n[SUCCESS] No cycles found in the prerequisite graph.\n");
                }
                break;
            case 3:
                addCourseToPlan(&student);
                break;
            case 4:
                viewStudentPlan(&student);
                break;
            case 5:
                printf("\nExiting planner. Good luck with your studies!\n");
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    } while (choice != 5);

    freeStudent(&student);
    return 0;
}

// Initialize sample courses with prerequisites, credits, semesters, and time slots
void initializeCatalog() {
    // Course 0: CS101
    strcpy(catalog[0].code, "CS101");
    strcpy(catalog[0].name, "Intro to Computer Science");
    catalog[0].credits = 4;
    catalog[0].semesterOffered = 1;
    catalog[0].timeSlot = 1;
    catalog[0].prereqCount = 0;

    // Course 1: CS102
    strcpy(catalog[1].code, "CS102");
    strcpy(catalog[1].name, "Data Structures & Algorithms");
    catalog[1].credits = 4;
    catalog[1].semesterOffered = 2;
    catalog[1].timeSlot = 2;
    strcpy(catalog[1].prerequisites[0], "CS101");
    catalog[1].prereqCount = 1;

    // Course 2: MA101
    strcpy(catalog[2].code, "MA101");
    strcpy(catalog[2].name, "Linear Algebra");
    catalog[2].credits = 3;
    catalog[2].semesterOffered = 1;
    catalog[2].timeSlot = 3;
    catalog[2].prereqCount = 0;

    // Course 3: AI201
    strcpy(catalog[3].code, "AI201");
    strcpy(catalog[3].name, "Artificial Intelligence Foundations");
    catalog[3].credits = 4;
    catalog[3].semesterOffered = 3;
    catalog[3].timeSlot = 1;
    strcpy(catalog[3].prerequisites[0], "CS102");
    strcpy(catalog[3].prerequisites[1], "MA101");
    catalog[3].prereqCount = 2;

    totalCourses = 4;
}

void displayCatalog() {
    printf("\n--- AVAILABLE COURSE CATALOG ---\n");
    for (int i = 0; i < totalCourses; i++) {
        printf("[%d] Code: %s | Name: %s\n", i, catalog[i].code, catalog[i].name);
        printf("    Credits: %d | Semester: %d | Time Slot: %d\n", 
               catalog[i].credits, catalog[i].semesterOffered, catalog[i].timeSlot);
        printf("    Prerequisites: ");
        if (catalog[i].prereqCount == 0) {
            printf("None\n");
        } else {
            for (int p = 0; p < catalog[i].prereqCount; p++) {
                printf("%s ", catalog[i].prerequisites[p]);
            }
            printf("\n");
        }
        printf("----------------------------------------\n");
    }
}

int findCourseIndex(const char *code) {
    for (int i = 0; i < totalCourses; i++) {
        if (strcmp(catalog[i].code, code) == 0) {
            return i;
        }
    }
    return -1;
}

bool isCourseCompleted(Student *s, const char *code) {
    for (int i = 0; i < s->completedCount; i++) {
        if (strcmp(s->completedCourses[i], code) == 0) {
            return true;
        }
    }
    return false;
}

// Helper DFS function for cycle detection in prerequisite adjacency structure
bool checkCycleUtil(int u, bool visited[], bool recStack[]) {
    visited[u] = true;
    recStack[u] = true;

    for (int i = 0; i < catalog[u].prereqCount; i++) {
        int v = findCourseIndex(catalog[u].prerequisites[i]);
        if (v != -1) {
            if (!visited[v] && checkCycleUtil(v, visited, recStack)) {
                return true;
            } else if (recStack[v]) {
                return true;
            }
        }
    }
    recStack[u] = false;
    return false;
}

bool hasCyclicPrerequisites() {
    bool visited[MAX_COURSES] = {false};
    bool recStack[MAX_COURSES] = {false};

    for (int i = 0; i < totalCourses; i++) {
        if (!visited[i]) {
            if (checkCycleUtil(i, visited, recStack)) {
                return true;
            }
        }
    }
    return false;
}

// Add Course to Semester Plan with Prerequisite, Time Slot, and Overload Checks
void addCourseToPlan(Student *s) {
    displayCatalog();
    char code[MAX_CODE_LEN];
    printf("\nEnter Course Code to add to your semester plan: ");
    scanf("%s", code);

    int idx = findCourseIndex(code);
    if (idx == -1) {
        printf("\n[ERROR] Course code '%s' not found in catalog!\n", code);
        return;
    }

    // 1. Check if already added in current plan
    for (int i = 0; i < s->plannedCount; i++) {
        if (s->plannedCourseIndices[i] == idx) {
            printf("\n[NOTICE] Course is already in your current semester plan.\n");
            return;
        }
    }

    // 2. Check if already completed previously
    if (isCourseCompleted(s, code)) {
        printf("\n[NOTICE] You have already completed this course previously!\n");
        return;
    }

    // 3. Check Prerequisites Validation
    for (int i = 0; i < catalog[idx].prereqCount; i++) {
        char *prereqCode = catalog[idx].prerequisites[i];
        if (!isCourseCompleted(s, prereqCode)) {
            printf("\n[PREREQUISITE ERROR] You have not completed prerequisite course: %s\n", prereqCode);
            printf("Suggestion: Complete '%s' first before enrolling in '%s'.\n", prereqCode, catalog[idx].code);
            return;
        }
    }

    // 4. Check Time Slot Overlap Conflicts
    for (int i = 0; i < s->plannedCount; i++) {
        int existingIdx = s->plannedCourseIndices[i];
        if (catalog[existingIdx].timeSlot == catalog[idx].timeSlot) {
            printf("\n[SCHEDULING CONFLICT] Time slot overlap detected!\n");
            printf("'%s' shares Time Slot %d with already planned course '%s'.\n", 
                   catalog[idx].code, catalog[idx].timeSlot, catalog[existingIdx].code);
            return;
        }
    }

    // 5. Dynamic memory check & reallocation for student plan
    if (s->plannedCount >= s->plannedCapacity) {
        s->plannedCapacity *= 2;
        int *temp = (int *)realloc(s->plannedCourseIndices, s->plannedCapacity * sizeof(int));
        if (temp == NULL) {
            printf("\n[MEMORY ERROR] Allocation failed!\n");
            exit(1);
        }
        s->plannedCourseIndices = temp;
    }

    // Add course index to plan
    s->plannedCourseIndices[s->plannedCount++] = idx;

    // Compute cumulative credits & check overload warning
    int totalCredits = 0;
    for (int i = 0; i < s->plannedCount; i++) {
        totalCredits += catalog[s->plannedCourseIndices[i]].credits;
    }

    printf("\n[SUCCESS] Course '%s' successfully added to your semester plan.\n", catalog[idx].code);
    printf("Current Cumulative Credits for Semester: %d\n", totalCredits);

    if (totalCredits > CREDIT_OVERLOAD_LIMIT) {
        printf("[WARNING] Credit overload warning! You have exceeded %d credits (%d credits total).\n", 
               CREDIT_OVERLOAD_LIMIT, totalCredits);
    }
}

void viewStudentPlan(Student *s) {
    printf("\n=========================================\n");
    printf("       ACADEMIC PLAN FOR: %s             \n", s->studentName);
    printf("=========================================\n");
    
    if (s->plannedCount == 0) {
        printf("No courses added to the current semester plan yet.\n");
        return;
    }

    int totalCredits = 0;
    for (int i = 0; i < s->plannedCount; i++) {
        int idx = s->plannedCourseIndices[i];
        printf("%d. [%s] %s | Credits: %d | Time Slot: %d\n", 
               i + 1, catalog[idx].code, catalog[idx].name, catalog[idx].credits, catalog[idx].timeSlot);
        totalCredits += catalog[idx].credits;
    }

    printf("-----------------------------------------\n");
    printf("Total Cumulative Credits: %d\n", totalCredits);
    if (totalCredits > CREDIT_OVERLOAD_LIMIT) {
        printf("[WARNING] Status: Credit Overload (%d credits > limit of %d)\n", 
               totalCredits, CREDIT_OVERLOAD_LIMIT);
    } else {
        printf("Status: Normal Workload\n");
    }
    printf("=========================================\n");
}

void freeStudent(Student *s) {
    if (s->plannedCourseIndices != NULL) {
        free(s->plannedCourseIndices);
        s->plannedCourseIndices = NULL;
    }
}

