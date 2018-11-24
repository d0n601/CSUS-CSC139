/*
 * CSC139 
 * Fall 2018
 * Second Assignment EXTRA CREDIT!
 * Kozak, Ryan
 * Section #02

 * OSs Tested on: Linux
 * 
 *
 * Architectures Tested on:
 *
 * Athena: Architecture:  i686
 * CPU op-mode(s):        32-bit, 64-bit
 * Byte Order:            Little Endian
 * CPU(s):                4
 * On-line CPU(s) list:   0-3
 * Thread(s) per core:    1
 * Core(s) per socket:    2
 * Socket(s):             2
 * Vendor ID:             GenuineIntel
 * CPU family:            6
 * Model:                 37
 * Model name:            Intel(R) Xeon(R) CPU E5-2640 0 @ 2.50GHz
 * Stepping:              1
 * CPU MHz:               2500.000
 * BogoMIPS:              5000.00
 * Hypervisor vendor:     VMware
 * Virtualization type:   full
 * L1d cache:             32K
 * L1i cache:             32K
 * L2 cache:              256K
 * L3 cache:              15360K
 *

 * */



#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <semaphore.h>

#define MAX_SIZE 100000000
#define MAX_THREADS 16
#define RANDOM_SEED 7654
#define MAX_RANDOM_NUMBER 5000

// Global variables
void* gShmPtr;
long gRefTime; //For timing
int gData[MAX_SIZE]; //The array that will hold the data
int indices[MAX_PROCESSES][3]; /* I set this, not in original template */
int gProcessCount; //Number of processes
int gDoneProcessCount; //Number of processes that are done at a certain point. Whenever a process is done, it increments this. Used with the semaphore-based solution
int gProcessMin[MAX_PROCESSES]; //The minimum value found by each thread
bool gProcessDone[MAX_PROCESSES]; //Is this Process done? Used when the parent is continually checking on child threads
// Semaphores
sem_t completed; //To notify parent that all processes have completed or one of them found a zero
sem_t mutex; //Binary semaphore to protect the shared variable gDoneProcessCount

void InitShm(int, int);
int SqFindMin(int size); //Sequential FindMin (no threads)
void *ThFindMin(void *param); //Process FindMin but without semaphores 
void *ThFindMinWithSemaphore(void *param); //Process FindMin with semaphores 
int SearchpRrocessMin(); // Search all process minima to find the minimum value found in all processes
void InitShm(int, int); 
void InitSharedVars();
void GenerateInput(int size, int indexForZero); //Generate the input array
void CalculateIndices(int arraySize, int prsCnt, int indices[MAX_PROCESSES[3]); //Calculate the indices to divide the array into T divisions, one division per process
int GetRand(int min, int max);//Get a random number between min and max

//Timing functions
long GetMilliSecondTime(struct timeb timeBuf);
long GetCurrentTime(void);
void SetTime(void);
long GetTime(void);

int main(int argc, char *argv[]){

	pid_t pid[MAX_THREADS];  
	//pthread_attr_t attr[MAX_THREADS];
	int indices[MAX_THREADS][3];
	int i, indexForZero, arraySize, min;

	// Code for parsing and checking command-line arguments
	if(argc != 4){
		fprintf(stderr, "Invalid number of arguments!\n");
		exit(-1);
	}
	if((arraySize = atoi(argv[1])) <= 0 || arraySize > MAX_SIZE){
		fprintf(stderr, "Invalid Array Size\n");
		exit(-1);				
	}
	gThreadCount = atoi(argv[2]);				
	if(gThreadCount > MAX_THREADS || gThreadCount <=0){
		fprintf(stderr, "Invalid Thread Count\n");
		exit(-1);				
	}
	indexForZero = atoi(argv[3]);
	if(indexForZero < -1 || indexForZero >= arraySize){
		fprintf(stderr, "Invalid index for zero!\n");
		exit(-1);
	}

	GenerateInput(arraySize, indexForZero);
	CalculateIndices(arraySize, gThreadCount, indices); 

	// Code for the sequential part
	SetTime();
	min = SqFindMin(arraySize);
	printf("Sequential search completed in %ld ms. Min = %d\n", GetTime(), min);
	

	// Threaded with parent waiting for all child threads
	InitSharedVars();
	SetTime();

	// Write your code here
	// Initialize threads, create threads, and then let the parent wait for all threads using pthread_join
	// The thread start function is ThFindMin
	// Don't forget to properly initialize shared variables 
       for (int i = 0; i < gThreadCount; i++) {
            pthread_create(&tid[i], NULL, ThFindMin, &indices[i]);
        }
        // Now the main thread waits for all the children to finish.
	for (int i = 0; i < gThreadCount; i++) {
            pthread_join(tid[i], NULL);
	}
        min = SearchThreadMin();
	printf("Multiprocess FindMin with parent waiting for all children completed in %ld ms. Min = %d\n", GetTime(), min);

	// Multi-process with busy waiting (parent continually checking on child processes without using semaphores)
	InitSharedVars();
	SetTime();

	// Write your code here
        // Don't use any semaphores in this part	
	// Initialize processes, create processess, and then make the parent continually check on all child processes
	// The thread start function is ThFindMin
	// Don't forget to properly initialize shared variables 
        
	for(int i = 0; i < gProcessCount; i++) {
          //  pthread_create(&tid[i], NULL, ThFindMin, &indices[i]);
        }
        
        // Parent checking on children
       volatile int weDone = 0; // We need a non critical variable to track finished children.

       // Loop until all the children are done, or one finds a zero.
       while(weDone < gProcessCount) {
            weDone = 0; // If we don'e reset this, one child finished will keep bumping it.
            for(int i = 0; i < gProcessCount; i ++) {
                if(gProcessDone[i]) {
                    if(gProcessMin[i] == 0) {
		        weDone = gProcessCount; // This breaks the loop constantly rechecking all children.
                        break; // Break inner child checking loop.
                    }
                    weDone++;
                }
            }
        }
        for(int i = 0; i < gProcessCount; i++) {
            pthread_cancel(tid[i]);
        } 
        min = SearchProcessMin();
	printf("Multiprocess FindMin with parent continually checking on children completed in %ld ms. Min = %d\n", GetTime(), min);
	

	// Multi-process with semaphores  
	InitSharedVars();

        // Initialize your semaphores here  
        sem_init(&mutex,0,1);
	sem_init(&completed,0,0);
	SetTime();

        // Write your code here
	// Initialize threads, create processes, and then make the parent wait on the "completed" semaphore 
	// The process start function is ThFindMinWithSemaphore
	// Don't forget to properly initialize shared variables and semaphores using sem_init 
        for (int i = 0; i < gProcessCount; i++) {
	    pthread_create(&tid[i], NULL, ThFindMinWithSemaphore, &indices[i]);
	}

	sem_wait(&completed);	//detect children processes completed
        
        for(int i = 0; i < gThreadCount; i++) {
	    pthread_cancel(tid[i]);
	}
	min = SearchThreadMin();
	printf("Threaded FindMin with parent waiting on a semaphore completed in %ld ms. Min = %d\n", GetTime(), min);
}


// Write a regular sequential function to search for the minimum value in the array gData
int SqFindMin(int size) {
   
    int min = gData[0]; // Assume first element is the smallest
   
    /* Loop through the array  */
    for (int i = 0; i < size; i++) {
        /* Return if we found 0 */
        if(gData[i] == 0) {
            return 0;
        }        

        /* If current element is smaller then min */
        else if (min > gData[i]) {
            min = gData[i]; // Replace min with current element
        }
    } 
    return min; // Return the smallest element.
}

// Write a process function that searches for the minimum value in one division of the array
// When it is done, this function should put the minimum in gProcessMin[processNum] and set gProcessDone[processNum] to true    
void* ThFindMin(void *param) {

    int processNum = ((int*)param)[0];
    int start = ((int*)param)[1];   
    int end = ((int*)param)[2];   // Get end array index for thread.

    for(int i = start; i < end; i++) {
        if(gData[i] < 1) {
	    gProcessMin[processNum] = 0; // This process found the zero.
            break;
        }
        else if(gData[i] < gProcessMin[processNum]) {
            gProcessMin[processNum] = gData[i]; // Set process min to current index in chunk. 
        }
    }
    gProcessDone[processdNum] = true; // Set this process to done. 
    pthread_exit(NULL); // exit process.*/
}

// Write a thread function that searches for the minimum value in one division of the array
// When it is done, this function should put the minimum in gThreadMin[threadNum]
// If the minimum value in this division is zero, this function should post the "completed" semaphore
// If the minimum value in this division is not zero, this function should increment gDoneThreadCount and
// post the "completed" semaphore if it is the last thread to be done
// Don't forget to protect access to gDoneThreadCount with the "mutex" semaphore     
void* ThFindMinWithSemaphore(void *param) {
   
    int processNum = ((int*)param)[0]; // Get thread number
    int start = ((int*)param)[1]; // Get start array index for thread.
    int end = ((int*)param)[2]; // Ged end array index for thread.

    for(int i = start; i < end; i++) {
        if(gData[i] < 1) {
            gProcessMin[processNum] = 0; // This process found the zero.
            sem_post(&completed); // Post the completed.
            break;
        }
        else if(gData[i] < gProcessMin[processNum]) {
            gProcessMin[threadNum] = gData[i]; // new smallest...ok
        }
    }    
 
    gProcessDone[threadNum] = true; // Set this thread to done.

    sem_wait(&mutex); // lock critical section
    
    gDoneProcessCount++; // edit global var.
    
    /* Check global var in critical section while it's locked */
    if (gDoneProcessCount == gProcessCount) {
        sem_post(&completed); // post complete if we're the last thread done.
    }
    
    sem_post(&mutex);  // Unlock the critical section.
    
    pthread_exit(NULL); // exit the thread now.

}

int SearchProcessMin() {
    int i, min = MAX_RANDOM_NUMBER + 1;
    for(i =0; i<gProcessCount; i++) {
        if(gProcessMin[i] == 0)
            return 0;
	if(gProcessDone[i] == true && gProcessMin[i] < min)
	    min = gProcessMin[i];
    }
    return min;
}

void InitSharedVars() {
    int i;
    for(i=0; i < gProcessCount; i++) {
        gProcessDone[i] = false;	
        gProcessMin[i] = MAX_RANDOM_NUMBER + 1;	
    }
    gDoneProcessCount = 0;
}

// Write a function that fills the gData array with random numbers between 1 and MAX_RANDOM_NUMBER
// If indexForZero is valid and non-negative, set the value at that index to zero 
void GenerateInput(int size, int indexForZero) {
    for(int c = 0; c < size; c++) {
        if(c == indexForZero) {
            gData[c] = 0;
        }
        else {
            gData[c] = GetRand(1, MAX_RANDOM_NUMBER);
        }
    }
}

// Write a function that calculates the right indices to divide the array into thrdCnt equal divisions
// For each division i, indices[i][0] should be set to the division number i,
// indices[i][1] should be set to the start index, and indices[i][2] should be set to the end index 
void CalculateIndices(int arraySize, int prsCnt, int indices[MAX_PROCESSES][3]) {
    
    int d = arraySize/prsCnt; 
    int start = 0;
    int end = d - 1;
   
     for (int i = 0; i < prsCnt; i++) {
	indices[i][0] = i;
	indices[i][1] = start;
	indices[i][2] = end; 
        start += d; //bump start index
        end += d; // bump end index.
    }

}


InitShm(int prsSize, int itemCnt)
{
    int in = 0;
    int out = 0;

    const char *name = "OS_HW2ec_ryanKozak"; // Name of shared memory object to be passed to shm_open

    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666); // Shared memory.

    // Check to ensure shared memory was created alright.
    if(shm_fd < 0) {
        printf("Error: Problem initilizing shared memory object.\n");
	exit(1);
    }

    // Truncate memory to 4k, and make sure that's successful.
    if(ftruncate(shm_fd, SHM_SIZE) < 0) { // Truncate memory object to 4k.
        printf("Error: Failed to truncate memory object.\n");
	exit(1);
    }
    // Get pointer to shared memory.

    gShmPtr =  mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 

		                                                                     			                                                                            if(gShmPtr < 0 ) {																					printf("Error: Failed to map virtual address space.\n");
	 exit(1);																				    }
//																						    SetIn(in);
    //SetOut(out);
//    SetBufSize(bufSize);
  //  SetItemCnt(itemCnt); 	
}

void SetHeaderVal(int i, int val)
{
    void* ptr = gShmPtr + i*sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}
int GetRand(int x, int y)
{
    int r = rand();
    r = x + r % (y-x+1);
    return r;
}
long GetMilliSecondTime(struct timeb timeBuf) {
    long mliScndTime;
    mliScndTime = timeBuf.time;
    mliScndTime *= 1000;
    mliScndTime += timeBuf.millitm;
    return mliScndTime;
}
long GetCurrentTime(void) {
    long crntTime=0;
    struct timeb timeBuf;
    ftime(&timeBuf);
    crntTime = GetMilliSecondTime(timeBuf);
    return crntTime;
}
void SetTime(void) {
    gRefTime = GetCurrentTime();
}
long GetTime(void) {
    long crntTime = GetCurrentTime();
    return (crntTime - gRefTime);
}

