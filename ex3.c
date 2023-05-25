// Adi Ben Yehuda 211769757

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

// #include <sys/stat.h>
// #include <fcntl.h>
// #include <ctype.h>
// #include <dirent.h>
// #include <sys/wait.h>
// #include <signal.h>

#define MAX_LINE_LENGTH 100 // Maximum length of a line

// typedef struct
// {
//     int producersID;
//     char *type;
//     int numOfsports;
//     int numOfNews;
//     int numOfweather;
// } Article;

typedef struct
{
    int id;
    int numOfProducts;
    int queueSize;
} Producer;

// For the Producer and the Co-Editors
typedef struct
{
    int size;
    char **buffer;         // Buffer array to store articles
    int front;             // first element in the queue.
    int rear;              // last element in the queue.
    pthread_mutex_t mutex; // Mutex for buffer access
    sem_t empty_count;     // Semaphore for empty slots in the buffer
    sem_t full_count;      // Semaphore for filled slots in the buffer
} BoundedBuffer;

// Constructor: create a new bounded buffer with size places to store objects.
char *initBoundedBuffer(BoundedBuffer *b, int size)
{
    b->size = size;
    b->front = -1;
    b->rear = -1;
    b->buffer = (char **)malloc(size * sizeof(char *));
    if (b->buffer == NULL)
    {
        perror("Memory allocation failed.\n");
        return NULL;
    }
    pthread_mutex_init(&b->mutex, NULL);
    sem_init(&b->empty_count, 0, size); // Initialize empty_count semaphore to size
    sem_init(&b->full_count, 0, 0);     // Initialize full_count semaphore to 0
}

// Insert a new object into the bounded buffer.
void insertArticle(BoundedBuffer *b, char *article)
{
    sem_wait(&b->empty_count);     // Decrement empty_count semaphore
    pthread_mutex_lock(&b->mutex); // Acquire the mutex lock

    // Check whether the queue is already full
    if (b->rear == b->size - 1)
        printf("Overflow \n");
    else
    {
        // When inserting the first element in the queue, set the value of Front to 0.
        if (b->front == -1)
            b->front = 0;

        b->rear = b->rear + 1;
        b->buffer[b->rear] = (char *)malloc(strlen(article) + 1);
        strcpy(b->buffer[b->rear], article);
    }

    pthread_mutex_unlock(&b->mutex); // Release the mutex lock
    sem_post(&b->full_count);        // Increment full_count semaphore
}

// Remove the first object from the bounded buffer and return it to the user
char *removeArticle(BoundedBuffer *b)
{
    sem_wait(&b->empty_count);     // Decrement empty_count semaphore
    pthread_mutex_lock(&b->mutex); // Acquire the mutex lock

    char *a;

    // Check whether the queue is already empty
    if (b->front == -1 || b->front > b->rear)
    {
        a="-1";
    }
    else
    {
        a = b->buffer[0];
        free(b->buffer[0]);
        for (int i = 1; i < b->rear; i++)
        {
            b->buffer[i - 1] = b->buffer[i];
        }
        b->rear--;
    }
    pthread_mutex_unlock(&b->mutex); // Release the mutex lock
    sem_post(&b->full_count);        // Increment full_count semaphore

    return a;
}

// Define a structure to hold the arguments
typedef struct
{
    Producer producer;
    BoundedBuffer *queue;
} ProducerThreadArgs;

void *thread_function_Producer(void *arg)
{
    ProducerThreadArgs *threadArgs = (ProducerThreadArgs *)arg;
    Producer producer = threadArgs->producer;
    BoundedBuffer *queue = threadArgs->queue;

    char *options[] = {"SPORTS", "NEWS", "WEATHER"};
    int numOptions = sizeof(options) / sizeof(options[0]);
    int countOptionByType[3] = {0};

    // Seed the random number generator
    srand(time(NULL));

    for (int i = 0; i < producer.numOfProducts; i++)
    {
        char article[30];
        strcpy(article, "Producer ");

        // Convert integer to string
        char idString[10];
        sprintf(idString, "%d", producer.id);
        strcat(article, idString);

        // Generate a random index within the range of options
        int randomIndex = rand() % numOptions;
        strcat(article, " ");
        strcat(article, options[randomIndex]);

        countOptionByType[randomIndex]++;
        strcat(article, " ");

        // Convert integer to string
        char countString[10];
        sprintf(countString, "%d", countOptionByType[randomIndex]);
        strcat(article, countString);

        printf("%s\n", article);
        insertArticle(queue, article);
    }

    char article[] = "DONE";
    insertArticle(queue, article);

    return NULL;
}

// Define a structure to hold the arguments
typedef struct
{
    BoundedBuffer *queuesOfArticals;
    BoundedBuffer * sportsQueue;
    BoundedBuffer * newsQueue;
    BoundedBuffer * weatherQueue;
    int numProducers;
} DispatcherThreadArgs;

void *thread_function_Dispatcher(void *arg)
{
    DispatcherThreadArgs *threadArgs = (DispatcherThreadArgs *)arg;
    BoundedBuffer *queuesOfArticals = threadArgs->queuesOfArticals;
    BoundedBuffer * sportsQueue = threadArgs->sportsQueue;
    BoundedBuffer * newsQueue = threadArgs->newsQueue;
    BoundedBuffer * weatherQueue = threadArgs->weatherQueue;

    int count = 0, numProducers = threadArgs->numProducers;
     printf("1");
    // When all the queues are empty, exit from the loop
    while (count != numProducers)
    {
        printf("1");
        // Get one article from each queue each time
        for (int i = 0; i < numProducers; i++)
        {
            printf("2");
            if (queuesOfArticals[i].size == 0) {
                continue;
            }

            char *article = removeArticle(&queuesOfArticals[i]);
            printf("%s", article);

            // Check if word is present in the string
            if (strstr(article, "SPORTS") != NULL)
            {
                // Add the article to the queue
                insertArticle(sportsQueue, article);
                printf("%s Inserted to the sportqueue", article);
            }
            else if (strstr(article, "NEWS") != NULL)
            {
                insertArticle(newsQueue, article);
                printf("%s Inserted to the newsqueue", article);

            }
            else if (strstr(article, "WEATHER") != NULL)
            {
                insertArticle(weatherQueue, article);
                printf("%s Inserted to the weatherqueue", article);

            } else if (strstr(article, "DONE") != NULL) {
                // This means the producer has finished inserting new articles
                count++; 

                // Remove the queue
                free(queuesOfArticals[i].buffer);
                queuesOfArticals[i].size = 0;
            }
        }
    }

    return NULL;
}

void *thread_function_Screen_manager(void *arg)
{
    // Thread code goes here
    return NULL;
}

int main(int argc, char *argv[])
{
    // if (argc < 2)
    // {
    //     exit(-1);
    // }

    // char *configPath = argv[1];
    char *configPath = "conf.txt";

    int numProducers = 0, capacity = 0, CoEditorSize = 0;
    Producer *producers = NULL;

    FILE *configFile = fopen(configPath, "r"); // Open the file in read mode
    if (configFile == NULL)
    {
        perror("Failed to open the file");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), configFile) != NULL)
    {
        Producer p;
        p.id = atoi(line);

        if (fgets(line, sizeof(line), configFile) != NULL)
        {
            p.numOfProducts = atoi(line);
        }
        else
        {
            CoEditorSize = atoi(line);
            break;
        }

        if (fgets(line, sizeof(line), configFile) != NULL)
        {
            p.queueSize = atoi(line);
        }

        fgets(line, sizeof(line), configFile);

        // Check if producers array is full, resize if needed
        if (numProducers >= capacity)
        {
            capacity = capacity == 0 ? 1 : capacity * 2;
            Producer *new_producers = (Producer *)calloc(capacity, sizeof(Producer));

            if (new_producers == NULL)
            {
                perror("Memory allocation failed");
                exit(1);
            }

            // Copy existing elements to the new array
            for (int i = 0; i < numProducers; i++)
            {
                new_producers[i].id = producers[i].id;
                new_producers[i].numOfProducts = producers[i].numOfProducts;
                new_producers[i].queueSize = producers[i].queueSize;
            }

            free(producers);
            producers = new_producers;
        }

        producers[numProducers] = p;
        numProducers++;
    }

    // Create an array of queue in the size of producers number with malloc
    BoundedBuffer *queuesOfArticals = (BoundedBuffer *)calloc(numProducers, sizeof(BoundedBuffer));

    for (int i = 0; i < numProducers; i++)
    {
        pthread_t thread_producer_id;

        BoundedBuffer queueOfArticals;
        initBoundedBuffer(&queueOfArticals, producers[i].queueSize);

        ProducerThreadArgs args;
        args.producer = producers[i];
        args.queue = &queueOfArticals;

        int result = pthread_create(&thread_producer_id, NULL, thread_function_Producer, (void *)&args);
        if (result != 0)
        {
            printf("pthread_create failed. Error code: %d\n", result);
            exit(1);
        }

        queuesOfArticals[i] = queueOfArticals;
    }

    pthread_t thread_producer_id;

    // Create 3 queues for each type
    BoundedBuffer sportsQueue;
    initBoundedBuffer(&sportsQueue, CoEditorSize);
    BoundedBuffer newsQueue;
    initBoundedBuffer(&newsQueue, CoEditorSize);
    BoundedBuffer weatherQueue;
    initBoundedBuffer(&weatherQueue, CoEditorSize);

    DispatcherThreadArgs dispatcherThreadArgs;
    dispatcherThreadArgs.queuesOfArticals = queuesOfArticals;
    dispatcherThreadArgs.sportsQueue = &sportsQueue;
    dispatcherThreadArgs.newsQueue = &newsQueue;
    dispatcherThreadArgs.weatherQueue = &weatherQueue;
    dispatcherThreadArgs.numProducers = numProducers;
    
    int result = pthread_create(&thread_producer_id, NULL, thread_function_Dispatcher, (void *)&dispatcherThreadArgs);
    if (result != 0)
    {
        printf("pthread_create failed. Error code: %d\n", result);
        exit(1);
    }

    free(producers);
    free(queuesOfArticals);

    // TODO: create function that free all the elements and call it when the malloc is failed.

    fclose(configFile); // Close the file
    return 0;
}