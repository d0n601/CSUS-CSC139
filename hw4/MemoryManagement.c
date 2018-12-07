/*
 * CSC139
 * Fall 2018
 * Fourth Assignment
 * Kozak, Ryan
 * Section #02
 * OSs Tested on: Linux
 *
 * Compile with: gcc -std=c99 -Wall ./MemoryManagement.c
 *
 * Expects an input.txt to be in the same directory
 * Outputs an output.txt in the same directory
 *
*/

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define F_MAX 2000
#define INPUT_FILE "/home/x/Desktop/tests/test5.txt"

int l1[3]; // Map: 0 pages, 1 frames, 2 requests
int rq[F_MAX] = {-1}; // Array of initial page requests.

// Variables for FIFO Queue.
int queue[F_MAX] = {-1};
int queue_size = 0;
int front = 0;
int rear = 0;

void ParseInput(); // Parse the input file and populate global vars.
int Insert(int p); // Insert integer into queue.
int Peek(); // Show the item at the head of the queue.
int Remove(); // Delete an element from the queue.
int Full(); // Is the queue full.
int LinearSearch(int element, int const* array, int s, int e); // Search the FIFO queue for an element.
void FirstInFirstOut();
void Optimal();
void LeastRecentlyUsed();



int main() {
    ParseInput();
    printf("\n");
    FirstInFirstOut();
    printf("\n");
    Optimal();
    printf("\n");
    LeastRecentlyUsed();
    return 0;
}


// First-In-First-Out (FIFO) Algorithm
void FirstInFirstOut() {

    printf("FIFO\n"); // Print algorithm name.

    int faults = 0; // Page fault count to zero.

    // Loop page requests
    for(int c = 0; c < l1[2]; c++) {

        // Find the requested page in the frames
        int frame = LinearSearch(rq[c], queue, front, rear )  % l1[1] ;

        // If the page is not in a frame.
        if(frame < 0) {

            // If the queue is full ( all frames full )
            if(Full() == 1) {
                int tp = Peek(); // Get page removed.
                frame = LinearSearch(tp, queue, front, rear ) % l1[1]; // Gate frame cleared.
                Remove(); // Remove page from queue.
                printf("Page %i unloaded from Frame %i, ", tp, frame); // Output that this happened.
            }
            Insert(rq[c]); // Insert the page into the queue.
            frame = LinearSearch(rq[c], queue, front, rear ) % l1[1]; // Get the frame we've inserted into.
            printf("Page %i loaded into Frame %i\n", rq[c], frame); // Output that this happened.
            faults++; // Increase page fault count.
        }
        else {
            printf("Page %i already in Frame %i\n", rq[c], frame); // Output that page is in the frame, no fault.
        }
    }

    printf("%i page faults\n", faults); // Output page fault fount.



}


// Optimal Policy Algorithm
void Optimal() {

    printf("Optimal\n"); // Print algorithm name.

    int faults = 0; // Page fault count to zero.
    int something = 0;
    int frames[F_MAX] = {-1}; // Frame array for optimal algorithm.

    /* Loop all page requests */
    for(int c = 0; c < l1[2]; c++) {

        // Find the requested page in the frames
        int frame = LinearSearch(rq[c], frames, 0, l1[1]);

        // If the page is not in a frame.
        if (frame < 0) {

            // If all frames full
            if(something >= l1[1]) {

                int q = INT_MAX, ln = INT_MIN, tc = 0;
                int temp[F_MAX] = {-1}; // Frame array for optimal algorithm.


                /* Loop frames */
                for(int i = 0; i < l1[1]; i++) {
                    int n = LinearSearch(frames[i], rq, c, l1[2]); // Is frame in remaining array.
                    // If infinite, this is the smallest frame number, so break.
                    if(n == -1) {
                        q = frames[i];
                        break;
                    }
                    // FAILING TEST 5, PASSING ALL OTHERS
                    else if(n > ln && LinearSearch(frames[i], temp, 0, l1[2]) < 0) {
                        ln = n;
                        q = frames[i]; // Page in frame.
                        temp[tc] = q; // Set page in temp array.
                        tc++; // Temp counter.
                    }

                }

                /* Replace Page */
                frame = LinearSearch(q, frames ,0, l1[1]);
                printf("Page %i unloaded from Frame %i, ", q, frame); // Output that this happened.
                frames[frame] = rq[c]; // Insert the page into the queue.
                printf("Page %i loaded into Frame %i\n", rq[c], frame); // Output that this happened.
                something--;
            }
            else {
                frames[something] = rq[c]; // Insert the page into the queue.
                printf("Page %i loaded into Frame %i\n", rq[c], something); // Output that this happened.
            }

            faults++; // Increase page fault count.
            something++;

        }
        else {
            printf("Page %i already in Frame %i\n", rq[c], frame); // Output that page is in the frame, no fault.
        }
    }

    printf("%i page faults\n", faults); // Output page fault fount.

}


// Least Recently Used Algorithm
void LeastRecentlyUsed() {
    printf("LRU\n");
}


// Parse the Input File.
void ParseInput() {

    FILE *fp;
    size_t len = 0;
    char *tmp;

    fp = fopen(INPUT_FILE, "r");

    if (fp == NULL) {
        exit(1);
    }

    getline(&tmp, &len, fp); // First input line.

    // Set global vars for pages, frames, and requests.
    for(int c = 0; c < 3; c++) {
        l1[c] = (int)strtol(strtok(tmp," "), &tmp, 10);
    }

    // Fill page requests array.
    for(int c = 0; c < l1[2]; c++) {
        getline(&tmp, &len, fp);
        rq[c] = (int)strtol(strtok(tmp," "), &tmp, 10);
    }

    fclose(fp);
}


// Insert page id into FIFO Queue.
int Insert(int p) {
    if(!Full()) {
        int i = rear;
        queue[rear++] = p;
        queue_size++;
        return i;
    }
    return -1;
}

// Remove page id from FIFO Queue.
int Remove() {
    if (front == rear) {
        return -1;
    }
    queue_size--;
    return queue[front++];
}


// Returns element next in queue.
int Peek() {
    return queue[front];
}


// Returns 1 if queue is full, 0 if not
int Full() {
    return queue_size >= l1[1];
}

// Performs linear search of an array, returns index or -1 if element not found.
int LinearSearch(int element, int const* array, int s, int e) {
    for (int cntr = s; cntr <= e; cntr++) {
        if (array[cntr] == element) {
            return cntr;
        }
    }
    return -1;
}
