/*
 * CSC139
 * Fall 2018
 * Third Assignment
 * Kozak, Ryan
 * Section #02
 * OSs Tested on: Linux
 *
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define P_MAX 2000
#define INPUT_FILE "./tests/input15.txt"

// Global Variables
int currp = -1;
int completed = 0;
int quantum = 0;
int clock = 0;
int pc = 0;
int queue_size = 0;

// Process number [0][y] Arrival Time [1][y], CPU burst time [2][y], Priority [3][y]
int process[4][P_MAX] = {-1};
int finished[3][P_MAX] = {0}; // This is to calculate AVG wait time more easily

char * p; // Character representation of pc.
char * alg; // Scheduling algorithm to use
char * tmp; // Temp buffer for converting numbers.

// Variables for FIFO Queue.
int queue[P_MAX] = {-1};
int front = 0;
int rear = 0;
int x = P_MAX;

void HandleArrivals(int process_number); // Handles processes that have just arrived.
float CalcWait();
void ClearQueue();
int GetBurst(int process_number);
void SetBurst(int process_number, int burst_time);
void Insert(int process_number); // Insert integer into queue.
int Remove(); // Delete an element from the queue.
void ParseInput(); // Parse the input file and populate global vars.
void SortProcesses(int s); // Sort process lines.
int LinearSearch(int element, int const* array, int s, int e); // Search the queue for an element.


int main(int argc, char *argv[]) {
    ParseInput(); // Scan the input file, setup vars, call algorithm.
    return 0;
}


// Round Robin
void RoundRobin() {

    quantum = (int)alg[3] - 48; // Time quantum.
    completed = 0;
    HandleArrivals(-1); // Input new processes into queue.
    printf("RR\n");

    while(completed < pc) {
        currp = Remove(); // Get process from stack.
        printf("%i %i \n", clock, currp); // Print the current clock time, and process number

        int b = GetBurst(currp); // Get the burst of the process

        // If the burst is greater than the time quantum
        if(b > quantum) {
            b -= quantum; // Subtract the time quantum from the burst.
            SetBurst(currp, b); // Set new burst after quantum expires.
            clock += quantum; // Advance the clock one time quantum.
            finished[2][currp] = finished[2][currp]+quantum;
            HandleArrivals(currp); // Handle processes that arrived just now.
            Insert(currp); // Insert process back into the end of the queue.
        }
        else if(b > 0 && b <= quantum) {
            HandleArrivals(currp); // Handle processes that arrived just now.
            SetBurst(currp, -1); // Set burst to negative.
            clock+= b; // Advance clock the time taken for this to finish.
            finished[2][currp] = finished[2][currp]+b;
            finished[0][currp] = clock; // Set finish time of process
            completed ++; // Increase count of completed processes.
        }
        else {
            clock ++;
            HandleArrivals(currp); // Handle processes that arrived just now.
        }

    }
    printf("AVG Waiting Time: %0.2f\n", CalcWait());
}

// Shortest Job First
void ShortestJobFirst() {

    SortProcesses(2); // Sort processes by shortest burst time
    completed = 0; // Set completed processes to zero.
    HandleArrivals(-1); // Input new processes into queue.

    printf("SJF\n"); // Output Line 1:

    while(completed < pc) {

        currp = Remove(); // Get process from stack.
        printf("%i %i \n", clock, currp); // Print the current clock time, and process number

        if(currp > 0) {
            int b = GetBurst(currp); // Get the burst of the process
            clock += b;
            SetBurst(currp, 0);
            finished[0][currp] = clock; // Set finish time of process.
            finished[2][currp] = b; // Set how much time process had CPU.
            completed++; // Increase count of completed processes.
            ClearQueue();
            HandleArrivals(currp); // Handle processes that arrived just now.
        }
        else {
            clock ++;
            ClearQueue();
            HandleArrivals(currp); // Handle processes that arrived just now.
        }

    }
    printf("AVG Waiting Time: %0.2f\n", CalcWait()); // Last Line of output
}

// Priority Scheduling without Preemption (PR_noPREMP)
void PR_noPremp() {

    SortProcesses(3); // Sort processes by priority (lower number =  higher priority).
    completed = 0;
    HandleArrivals(-1); // Input new processes into queue.
    printf("PR_noPREMP\n");

    while(completed < pc) {

        currp = Remove(); // Get process from stack.
        printf("%i %i \n", clock, currp); // Print the current clock time, and process number.

        if(currp > 0) {
            int b = GetBurst(currp); // Get the burst of the process.
            clock += b;
            SetBurst(currp, 0);
            finished[0][currp] = clock; // Set finish time of process.
            finished[2][currp] = b; // Set how much time process had CPU.
            completed++; // Increase count of completed processes.
            ClearQueue();
            HandleArrivals(currp); // Handle processes that arrived just now.
        }
        else {
            clock ++;
            ClearQueue();
            HandleArrivals(currp); // Handle processes that arrived just now.
        }
    }
    printf("AVG Waiting Time: %0.2f\n", CalcWait());
}

// Priority Scheduling with Preemption (PR_withPREMP)
void PR_withPremp() {

    SortProcesses(3); // Sort processes by priority (lower number =  higher priority).
    completed = 0;
    HandleArrivals(-1); // Input new processes into queue.
    printf("PR_withPREMP\n");


    while(completed < pc) {

        int lastp = currp;

        currp = Remove(); // Get process from stack.

        if(currp != lastp) {
            printf("%i %i \n", clock, currp); // Print the current clock time, and process number.
        }

        clock ++;

        if(currp > 0) {

            int b = GetBurst(currp); // Get the burst of the process.
            b--;
            finished[2][currp] = finished[2][currp]+1;
            SetBurst(currp, b);

            if(b < 1) {
                finished[0][currp] = clock; // Set finish time of process.
                completed++; // Increase count of completed processes.
            }
        }
        ClearQueue();
        HandleArrivals(-1); // Handle processes that arrived just now.
    }
    printf("AVG Waiting Time: %0.2f\n", CalcWait());
}

// Parse the Input File.
void ParseInput() {

    FILE *fp;
    size_t len = 0;
    char *buffer, *pch;

    fp = fopen(INPUT_FILE, "r");

    if (fp == NULL) {
        exit(1);
    }

    getline(&alg, &len, fp); // Read algorithm
    getline(&p, &len, fp); // Read number of processes

    pc = (int)strtol(p, &tmp, 10);

    // Loop through the process lines and populate process array vars.
    for(int c = 0; c < pc; c++) {
        getline(&buffer, &len, fp); // Get the process line
        pch = strtok(buffer," ");
        for(int x = 0; x < 4; x++) {
            if(pch == NULL) {
                exit(1);
            }
            process[x][c] = (int)strtol(pch, &tmp, 10);
            if(x == 1) {
                finished[1][process[0][c]] = process[x][c]; // Set processes start time in finish array.
            }
            pch = strtok(NULL, " ");
        }
    }

    // Call scheduling algorithm.
    if(alg[0] == 'R'){
        RoundRobin();
    }
    else if(alg[0] == 'S') {
        ShortestJobFirst();
    }
    else if(alg[0] == 'P' && alg[3] == 'n' ) {
        PR_noPremp();
    }
    else if(alg[0] == 'P' && alg[3] == 'w') {
        PR_withPremp();
    }
    else {
        exit(1);
    }
    fclose(fp);
    exit(0);
}

// Calculate the average wait time.
float CalcWait() {
    float w = 0;
    for(int cnt = 0; cnt < pc; cnt++) {
        int proc = process[0][cnt];
        int a,b,c;
        a = finished[0][proc];
        b = finished[1][proc];
        c = finished[2][proc];
        w += a-b-c;
    }
    return w/pc;
}

// Gets the remaining burst time for a process
int GetBurst(int process_number) {
    for(int cntr = 0; cntr < pc; cntr++) {
        if(process[0][cntr] == process_number) {
            return process[2][cntr];
        }
    }
    return -1;
}

// Sets the remaining burst time for a process
void SetBurst(int process_number, int burst_time) {
    for(int cntr = 0; cntr < pc; cntr++) {
        if(process[0][cntr] == process_number) {
            process[2][cntr] = burst_time;
            break;
        }
    }
}

// Performs linear search of an array, returns index or -1 if element not found.
int LinearSearch(int element, int const* array, int s, int e) {
    for(int cntr = s; cntr <= e; cntr++) {
        if(array[cntr] == element) {
            return cntr;
        }
    }
    return -1;
}

// Add new processes to the queue as they arrive, exclude specified process.
void HandleArrivals(int process_number) {
    // Loop our processes
    for(int cntr = 0; cntr < pc; cntr++) {
        // If the process has arrived at this point, and still has time to use
        if(process[0][cntr] != process_number && process[2][cntr] > 0 && process[1][cntr] <= clock) {
            // If it's not already in the queue
            if (LinearSearch(process[0][cntr], queue, front, rear) < 0) {
                Insert(process[0][cntr]); // Insert process number into queue.
            }
        }
    }

}

// Sorts process lines by parameter
void SortProcesses(int s){
    for(int i = 0; i < pc; i++) {
        for(int j = i + 1; j < pc; ++j) {
            if(process[s][i] > process[s][j]) {
                for(int cnt = 0; cnt < 4; cnt++) {
                    int temp =  process[cnt][i];
                    process[cnt][i] = process[cnt][j];
                    process[cnt][j] = temp;
                }
            }
        }
    }
    if(s == 2) {
        // Break ties in burst length by arrival time.
        for (int i = 0; i < pc; ++i) {
            for (int j = i + 1; j < pc; ++j) {
                if ((process[2][i] == process[2][j]) && (process[1][i] >= process[1][j])) {
                    for (int cnt = 0; cnt < 4; cnt++) {
                        int temp = process[cnt][i];
                        process[cnt][i] = process[cnt][j];
                        process[cnt][j] = temp;
                    }
                }
            }
        }
    }
    if(s == 3) {
        // Break ties in burst length by arrival time.
        for (int i = 0; i < pc; ++i) {
            for (int j = i + 1; j < pc; ++j) {
                if ((process[3][i] == process[3][j]) && (process[1][i] >= process[1][j])) {
                    for (int cnt = 0; cnt < 4; cnt++) {
                        int temp = process[cnt][i];
                        process[cnt][i] = process[cnt][j];
                        process[cnt][j] = temp;
                    }
                }
            }
        }
    }
}

// Clear the queue.
void ClearQueue() {
    front = 0; rear = 0;
    for(int cnt = 0; cnt < 11; cnt++) {
        queue[cnt] = -1;
    }
}

// Insert process id into FIFO Queue.
void Insert(int process_number) {
    if(rear != x) {
        queue[rear++] = process_number;
        queue_size++;
    }
}

// Remove process id from FIFO Queue.
int Remove() {
    if(front == rear) {
        return -1;
    }
    queue_size--;
    return queue[front++];
}
