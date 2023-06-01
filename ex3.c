// Adi Ben Yehuda 211769757

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define MAX_LINE_LENGTH 100 // Maximum length of a line

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
    int front;             // First element in the queue.
    int rear;              // Last element in the queue.
    pthread_mutex_t mutex; // Mutex for buffer access
    sem_t empty_count;     // Semaphore for empty slots in the buffer
    sem_t full_count;      // Semaphore for filled slots in the buffer
} BoundedBuffer;

// Constructor: create a new bounded buffer with size places to store objects.
void initBoundedBuffer(BoundedBuffer *b, int size)
{
    b->size = size;
    b->front = -1;
    b->rear = -1;
    b->buffer = (char **)malloc(size * sizeof(char *));
    if (b->buffer == NULL)
    {
        perror("Memory allocation failed.\n");
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            b->buffer[i] = NULL; // Initialize buffer elements to NULL
        }
    }
    pthread_mutex_init(&b->mutex, NULL);
    sem_init(&b->empty_count, 0, size); // Initialize empty_count semaphore to size
    sem_init(&b->full_count, 0, 0);     // Initialize full_count semaphore to 0
}

// Insert a new object into the bounded buffer.
void insertToBoundedBuffer(BoundedBuffer *b, char *article)
{
    bool insert = false;

    while (insert == false)
    {
        sem_wait(&b->empty_count);     // Decrement empty_count semaphore
        pthread_mutex_lock(&b->mutex); // Acquire the mutex lock

        // Check whether the queue isn't full
        if (b->rear < b->size - 1)
        {
            // When inserting the first element in the queue, set the value of Front to 0.
            if (b->front == -1)
                b->front = 0;

            b->rear = b->rear + 1;
            b->buffer[b->rear] = (char *)malloc(strlen(article) + 1);
            if (b->buffer[b->rear] == NULL)
            {
                perror("Memory allocation failed");
                pthread_mutex_unlock(&b->mutex); // Release the mutex lock
                sem_post(&b->full_count);        // Increment full_count semaphore
                exit(1);
            }
            strcpy(b->buffer[b->rear], article);
            insert = true;
        }

        pthread_mutex_unlock(&b->mutex); // Release the mutex lock
        sem_post(&b->full_count);        // Increment full_count semaphore
    }
}

// Remove the first object from the bounded buffer and return it to the user
char *removeFromBoundedBuffer(BoundedBuffer *b)
{
    pthread_mutex_lock(&b->mutex); // Acquire the mutex lock
    char *a;

    // Check whether the queue is already empty
    if (b->rear == -1)
    {
        a = NULL;
    }
    else
    {
        // Check whether the array is already empty
        if (b->buffer == NULL)
        {
            a = NULL;
        }
        else
        {
            a = b->buffer[0];
            for (int i = 1; i <= b->rear; i++)
            {
                b->buffer[i - 1] = b->buffer[i];
            }
            b->rear--;
        }
    }

    pthread_mutex_unlock(&b->mutex); // Release the mutex lock
    sem_post(&b->empty_count);       // Increment empty_count semaphore

    return a;
}

typedef struct Node
{
    char *data;
    struct Node *next;
} Node;

typedef struct
{
    Node *head;
    int size;
} LinkedList;

// For the Dispatcher
typedef struct
{
    LinkedList *buffer;    // Buffer linked list to store articles
    pthread_mutex_t mutex; // Mutex for buffer access
    sem_t full_count;      // Semaphore for filled slots in the buffer
} UnboundedBuffer;

// Function to create an empty linked list
LinkedList *createLinkedList()
{
    LinkedList *list = malloc(sizeof(LinkedList));
    if (list == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    list->head = NULL;
    list->size = 0;
    return list;
}

// Constructor: create a new unbounded buffer.
void initUnboundedBuffer(UnboundedBuffer *b, int size)
{
    b->buffer = createLinkedList();
    pthread_mutex_init(&b->mutex, NULL);
    sem_init(&b->full_count, 0, 0); // Initialize full_count semaphore to 0
}

// Function to add an element to the end of the linked list
void addEnd(UnboundedBuffer *b, char *data)
{
    Node *newNode = malloc(sizeof(Node));
    if (newNode == NULL)
    {
        perror("Memory allocation failed");
        return;
    }
    newNode->data = strdup(data);
    newNode->next = NULL;

    pthread_mutex_lock(&b->mutex); // Acquire the mutex lock

    // Check if the linked list is empty
    if (b->buffer->head == NULL)
    {
        b->buffer->head = newNode;
    }
    else
    {
        Node *current = b->buffer->head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newNode;
    }

    b->buffer->size++;
    pthread_mutex_unlock(&b->mutex); // Release the mutex lock
    sem_post(&b->full_count);        // Increment full_count semaphore
}

// Function to remove the first element from the linked list
char *removeFront(UnboundedBuffer *b)
{
    pthread_mutex_lock(&b->mutex); // Acquire the mutex lock

    // Check if the linked list is empty
    if (b->buffer->head == NULL)
    {
        pthread_mutex_unlock(&b->mutex); // Release the mutex lock
        return NULL;
    }

    Node *temp = b->buffer->head;
    char *article = temp->data;
    b->buffer->head = b->buffer->head->next;
    free(temp);
    b->buffer->size--;

    pthread_mutex_unlock(&b->mutex); // Release the mutex lock

    return article;
}

// Function to free the memory allocated for the linked list
void destroyLinkedList(LinkedList *list)
{
    Node *current = list->head;
    while (current != NULL)
    {
        Node *temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    free(list);
}

// Global variables
BoundedBuffer *queuesOfArticals;
UnboundedBuffer sportsQueue;
UnboundedBuffer newsQueue;
UnboundedBuffer weatherQueue;
BoundedBuffer screenManagerQueue;

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

        insertToBoundedBuffer(queue, article);
    }

    char article[] = "DONE";
    insertToBoundedBuffer(queue, article);

    free(arg); // Free the dynamically allocated memory

    return NULL;
}

// Define a structure to hold the arguments
typedef struct
{
    int numProducers;
} DispatcherThreadArgs;

void *thread_function_Dispatcher(void *arg)
{
    DispatcherThreadArgs *args = (DispatcherThreadArgs *)arg;
    int numProducers = args->numProducers;
    int count = 0;

    // When all the queues are empty, exit from the loop
    while (count != numProducers)
    {
        // Get one article from each queue each time
        for (int i = 0; i < numProducers; i++)
        {
            if (queuesOfArticals[i].size == 0)
            {
                continue;
            }

            char *article = removeFromBoundedBuffer(&queuesOfArticals[i]);
            // Check if the queue was empty
            if (article == NULL)
            {
                continue;
            }

            // Check if word is present in the string
            if (strstr(article, "SPORTS") != NULL)
            {
                addEnd(&sportsQueue, article);
            }
            else if (strstr(article, "NEWS") != NULL)
            {
                addEnd(&newsQueue, article);
            }
            else if (strstr(article, "WEATHER") != NULL)
            {
                addEnd(&weatherQueue, article);
            }
            else if (strstr(article, "DONE") != NULL)
            {
                // This means the producer has finished inserting new articles
                count++;

                // Remove the queue
                free(queuesOfArticals[i].buffer);
                queuesOfArticals[i].size = 0;
            }

            free(article);
        }
    }

    addEnd(&sportsQueue, "DONE");
    addEnd(&newsQueue, "DONE");
    addEnd(&weatherQueue, "DONE");

    free(args);

    return NULL;
}

// Define a structure to hold the arguments
typedef struct
{
    UnboundedBuffer *queue;
} CoEditorThreadArgs;

void *thread_function_Co_Editor(void *arg)
{
    CoEditorThreadArgs *args = (CoEditorThreadArgs *)arg;
    UnboundedBuffer *queue = args->queue;

    char *article;

    do
    {
        article = removeFront(queue);

        if (article == NULL)
        {
            continue;
        }
        else if (strstr(article, "DONE") != NULL)
        {
            // Check if the article is DONE
            insertToBoundedBuffer(&screenManagerQueue, article);
            free(article);

            break;
        }

        usleep(100000); // 0.1 seconds for editing
        insertToBoundedBuffer(&screenManagerQueue, article);

        free(article);
    } while (1);

    return NULL;
}

void *thread_function_Screen_manager(void *arg)
{
    int count = 0;

    while (count != 3)
    {
        char *article = removeFromBoundedBuffer(&screenManagerQueue);
        if (article == NULL)
        {
            continue;
        }
        else if (strstr(article, "DONE") != NULL)
        {
            // Check if the article is DONE
            count++;
            continue;
        }

        printf("%s\n", article);
        free(article);
    }

    printf("DONE\n");

    return NULL;
}

void freeQueuesOfArticals()
{
    for (int i = 0; i < queuesOfArticals->size; i++)
    {
        free(queuesOfArticals[i].buffer);
    }

    free(queuesOfArticals);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        exit(-1);
    }

    char *configPath = argv[1];

    int numProducers = 0, capacity = 0, CoEditorSize = 0, result = 0;
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
        p.id = atoi(line) - 1;

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
            if (atoi(line) <= 0)
            {
                printf("The configuration file is invalid\n");
                exit(1);
            }
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

    fclose(configFile); // Close the file

    // Create an array of queue in the size of producers number with malloc
    queuesOfArticals = (BoundedBuffer *)calloc(numProducers, sizeof(BoundedBuffer));
    if (queuesOfArticals == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for queuesOfArticals\n");
        return 1;
    }

    for (int i = 0; i < numProducers; i++)
    {
        initBoundedBuffer(&(queuesOfArticals[i]), producers[i].queueSize + 1);
        pthread_t thread_producer_id;

        ProducerThreadArgs *args = malloc(sizeof(ProducerThreadArgs));
        if (args == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for ProducerThreadArgs\n");
            exit(1);
        }
        args->producer = producers[i];
        args->queue = &(queuesOfArticals[i]);

        int result = pthread_create(&thread_producer_id, NULL, thread_function_Producer, (void *)args);
        if (result != 0)
        {
            printf("pthread_create failed. Error code: %d\n", result);
            exit(1);
        }
    }

    // Create 3 queues for each type
    initUnboundedBuffer(&sportsQueue, CoEditorSize);
    initUnboundedBuffer(&newsQueue, CoEditorSize);
    initUnboundedBuffer(&weatherQueue, CoEditorSize);

    pthread_t thread_dispatcher_id;
    DispatcherThreadArgs *dispatcherThreadArgs = (DispatcherThreadArgs *)malloc(sizeof(DispatcherThreadArgs));
    if (dispatcherThreadArgs == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for queuesOfArticals\n");
        exit(1);
    }
    dispatcherThreadArgs->numProducers = numProducers;

    result = pthread_create(&thread_dispatcher_id, NULL, thread_function_Dispatcher, (void *)dispatcherThreadArgs);
    if (result != 0)
    {
        printf("pthread_create failed. Error code: %d\n", result);
        exit(1);
    }

    initBoundedBuffer(&screenManagerQueue, CoEditorSize);

    pthread_t thread_sports_id;
    CoEditorThreadArgs coEditorSportsThreadArgs;
    coEditorSportsThreadArgs.queue = &sportsQueue;
    result = pthread_create(&thread_sports_id, NULL, thread_function_Co_Editor, (void *)&coEditorSportsThreadArgs);
    if (result != 0)
    {
        printf("pthread_create failed. Error code: %d\n", result);
        exit(1);
    }

    pthread_t thread_news_id;
    CoEditorThreadArgs coEditorNewsThreadArgs;
    coEditorNewsThreadArgs.queue = &newsQueue;
    result = pthread_create(&thread_news_id, NULL, thread_function_Co_Editor, (void *)&coEditorNewsThreadArgs);
    if (result != 0)
    {
        printf("pthread_create failed. Error code: %d\n", result);
        exit(1);
    }

    pthread_t thread_weather_id;
    CoEditorThreadArgs coEditorWeatherThreadArgs;
    coEditorWeatherThreadArgs.queue = &weatherQueue;
    result = pthread_create(&thread_weather_id, NULL, thread_function_Co_Editor, (void *)&coEditorWeatherThreadArgs);
    if (result != 0)
    {
        printf("pthread_create failed. Error code: %d\n", result);
        exit(1);
    }

    pthread_t thread_screen_manager_id;
    result = pthread_create(&thread_screen_manager_id, NULL, thread_function_Screen_manager, NULL);
    if (result != 0)
    {
        printf("pthread_create failed. Error code: %d\n", result);
        exit(1);
    }

    if (pthread_join(thread_screen_manager_id, NULL) != 0)
    {
        fprintf(stderr, "Failed to join thread\n");
        return 1;
    }

    free(producers);
    freeQueuesOfArticals();

    destroyLinkedList(sportsQueue.buffer);
    destroyLinkedList(newsQueue.buffer);
    destroyLinkedList(weatherQueue.buffer);

    free(screenManagerQueue.buffer);

    return 0;
}