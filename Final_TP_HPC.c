#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>
#include <time.h>

// Global variables
int num_jobs, num_tasks, num_machines;
int max_time = 0;
#define MAX_SEARCH 500000
#define FILE_NAME "jobshop3.txt"
#define NUM_THREADS 4

typedef struct {
    int machine_number;
    int time_need;
    int job_number;
    int task_number;
} Task;

// Function to transpose the matrix
void transposeMatrix(Task src[][num_tasks], Task dest[][num_jobs], int num_jobs, int num_tasks) {
    for (int i = 0; i < num_jobs; i++) {
        for (int j = 0; j < num_tasks; j++) {
            dest[j][i] = src[i][j];
        }
    }
}

// Function to print the transposed task matrix
void printMatrix(Task matrix[][num_jobs], int num_jobs, int num_tasks) {
    printf("[%d,%d]\n", num_jobs, num_tasks);
    printf("[\n");
    for (int i = 0; i < num_tasks; i++) {
        printf("[");
        for (int j = 0; j < num_jobs; j++) {
            printf("[%d,%d,%d,%d]", matrix[i][j].machine_number, matrix[i][j].time_need, matrix[i][j].job_number, matrix[i][j].task_number);
            if (j < num_jobs - 1) {
                printf(",");
            }
        }
        printf("]\n");
    }
    printf("]\n");
}

// Function to shuffle an array
int* shuffleArray(int arr[], int n, int seed) {
    srand(seed * seed);
    // Alocar memória para o novo array
    int* shuffledArr = (int*)malloc(n * sizeof(int));
    if (shuffledArr == NULL) {
        printf("Error creating array\n");
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        shuffledArr[i] = arr[i];
    }

    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = shuffledArr[i];
        shuffledArr[i] = shuffledArr[j];
        shuffledArr[j] = temp;
    }

    return shuffledArr;
}

// Initialize Schedules
void initialize_machines(char machines[num_machines][max_time][20]) {
    for (int i = 0; i < num_machines; i++) {
        for (int j = 0; j < max_time; j++) {
            strcpy(machines[i][j], "Empty");
        }
    }
}

// Function to create a schedule
void schedule_tasks(Task tasks[num_jobs][num_tasks], char machines[num_machines][max_time][20], int order[num_tasks]) {
    int job_start[100] = {0};
    for (int idx1 = 0; idx1 < num_tasks; idx1++) {
        for (int o = 0; o < num_jobs; o++) 
        {
            int idx2 = order[o];
            int job_num = tasks[idx1][idx2].job_number;
            int machine_num = tasks[idx1][idx2].machine_number;
            int time_need = tasks[idx1][idx2].time_need;
            int time_frame = job_start[job_num];
            int have_time = 0;
            while (!have_time) 
            {
                have_time = 1;
                for (int j = 0; j < time_need; j++) 
                {
                    if (strcmp(machines[machine_num][time_frame + j], "Empty") != 0) 
                    {
                        have_time = 0;
                        break;
                    }
                }
                if (!have_time) 
                {
                    time_frame++;
                }
            }
            for (int j = 0; j < time_need; j++) {
                sprintf(machines[machine_num][time_frame + j], "Job %d Task %d", job_num, idx1+1);
            }
            job_start[job_num] = time_frame + time_need; // Update start_time for next task
        }
    }
}

// Function to calculate the total time of a schedule
int calculate_total_time(char schedule[num_machines][max_time][20]) {
    int total_time = 0;
    for (int i = 0; i < num_machines; i++) {
        for (int j = max_time - 1; j >= 0; j--) {
            if (strcmp(schedule[i][j], "Empty") != 0) {
                if (j + 1 > total_time) {
                    total_time = j + 1;
                }
                break;
            }
        }
    }
    return total_time;
}

// Function to print the schedule
void print_machines(char machines[num_machines][max_time][20], int best_time) {
    for (int i = 0; i < num_machines; i++) {
        printf("\nMachine %d\n", i);
        for (int j = 0; j < best_time; j++) {
            printf("%d: %s\n", j + 1, machines[i][j]);
        }
    }
}

int main() {

    FILE *fp;
    int i, j;

    // Open file
    fp = fopen(FILE_NAME, "r");

    // Read first line
    fscanf(fp, "%d %d", &num_machines, &num_jobs);
    num_tasks = num_jobs;

    // Create matrix to store data
    Task orgData[num_jobs][num_tasks];

    // Read the file and stores the data in the matrix
    for (i = 0; i < num_jobs; i++) {
        for (j = 0; j < num_tasks; j++) {
            fscanf(fp, "%d %d", &orgData[i][j].machine_number, &orgData[i][j].time_need);
            orgData[i][j].job_number = i;
            orgData[i][j].task_number = j;
        }
    }

    // Close the file
    fclose(fp);

    // Create a transpose of the matrix
    Task tranData[num_tasks][num_jobs];
    transposeMatrix(orgData, tranData, num_jobs, num_tasks);

    // Print the data
    printMatrix(tranData, num_jobs, num_tasks);

    // Calculate max time as the sum of all task times
    for (int i = 0; i < num_jobs; i++) {
        for (int j = 0; j < num_jobs; j++) {
            max_time += tranData[i][j].time_need;
        }
    }

    printf("Max time: %d\n", max_time);

    // Create the order array
    int order[num_jobs];
    for (i = 0; i < num_jobs; i++) {
        order[i] = i;
    }

    // Look for a schedule
    char best_schedules[num_machines][max_time][20];
    int best_time = max_time;


    // Start time counter
    clock_t start_time = clock();

     // MT Schedule Search
    #pragma omp parallel for private(i)
    for (i = 0; i < MAX_SEARCH; i++) {
        char schedule[num_machines][max_time][20];
        initialize_machines(schedule);
        int* shuffledArr = shuffleArray(order, num_jobs, i);
        schedule_tasks(tranData, schedule, shuffledArr);
        int new_time = calculate_total_time(schedule);
        
        //printf("\n  %d - i=%d", new_time, i);

        #pragma omp critical
        {
            if (new_time < best_time) {
                best_time = new_time;
                memcpy(best_schedules, schedule, sizeof(schedule));
                
            }
        }
    }


    // End time counter
    clock_t end_time = clock();

    // Print the best found schedule
    print_machines(best_schedules, best_time);
    printf("\nBest time found: %d\n", best_time);

    // Print the time taken for the search
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time taken for search: %f seconds\n", time_taken);
    printf("Search runs: %d seconds\n", MAX_SEARCH);


    return 0;
}