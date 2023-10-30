# Threads Synchronization

## Introduction
The purpose of this project is to gain experience with concurrent programming and synchronization mechanisms.
The overall scenario we are simulating is that of news broadcasting. Different types of stories are produced and the system sorts them and displays them to the public.

## Table of contents
* [General Information](#general-information)
* [Installation](#installation)

## General Information
In this assignment, the ‘new stories’ are simulated by simple strings which should be displayed on the screen in the order they arrive.
In the scenario, there are 4 types of active actors: 

- **<ins> Producer:</ins>** Each producer creates some strings in the following format: "producer i type j"  where ‘i’ is the producer's ID, ‘type' is a random type it chooses which can be ‘SPORTS’, ‘NEWS’, 'WEATHER’, and ‘j’ is the number of strings of type ‘type’ this producer has already produced. Each of the producers passes its information to the Dispatcher via its private queue. Each of the Producer's private queue is shared between the Producer and the Dispatcher. Each of the string products is inserted by the Producer into its ‘producers queue’. After inserting all the products, the Producer sends a ‘DONE’ string through its Producer queue.

  For example,
![image](https://github.com/adi-ben-yehuda/operating_ex3/assets/75027826/4d29c789-a607-4413-8f0c-9e759a171307)

- **<ins> Dispatcher:</ins>** The Dispatcher continuously accepts messages from the Producers' queues. It scans the Producer's queue using a Round Robin algorithm. The Dispatcher does not block when the queues are empty. Each message is "sorted" by the Dispatcher and inserted into one of the Dispatcher queues which includes strings of a single type. When the Dispatcher receives a "DONE" message from all Producers, it sends a "DONE" message through each of its queues.

- **<ins> Co-Editors:</ins>** For each type of possible message there is a Co-Editor that receives the message through the Dispatchers queue, "edits" it, and passes it to the screen manager via a single shared queue. The editing process will be simulated by the Co-Editors by blocking for one-tenth (0.1) of a second. When a Co-Editor receives a "DONE" message, it passes it without waiting through the shared queue.

- **<ins> Screen-manager:</ins>** The Screen-manager displays the strings it receives via the Co-Editors queue to the screen. After printing all messages to the screen and receiving three "DONE" messages, the Screen manager displays a ‘DONE’ statement.

### System Design
The system is implemented as follows:

![image](https://github.com/adi-ben-yehuda/operating_ex3/assets/75027826/d6c552fb-242a-4bc7-adc0-5303cfd50625)

Three producers communicate with the dispatcher via their Producer queues. The Dispatcher communicates with the Co-Editors via three queues corresponding to the three types of messages. The Co-Editors communicate with the screen manager via a single shared queue, and the Screen manager displays the system output.

**<ins> Bounded Buffer:</ins>**

The Producer queues in this assignment and the Co-Editors' shared queue are a bounded buffer that supports the following operations.
- Bounded_Buffer (int size) – (constructor) creates a new bounded buffer with size places to store objects.
- void insert (char * s) – insert a new object into the bounded buffer. 
- char * remove ( ) - Remove the first object from the bounded buffer and return it to the user.

We implement a thread-safe bounded buffer using a binary semaphore (mutex) and create a counting semaphore with two binary semaphores. 

### Configuration File 
The Configuration file has the following format: 

![image](https://github.com/adi-ben-yehuda/operating_ex3/assets/75027826/12136ce1-2c97-42eb-92b1-7ebb98b96e07)

For example, 

![image](https://github.com/adi-ben-yehuda/operating_ex3/assets/75027826/746c0d70-95ca-4b76-9976-55de3108156a)

## Installation
Before installing this project, you need to install on your computer:
* Git

Then open a terminal.
write the next command:
```
git clone https://github.com/adi-ben-yehuda/Threads-Synchronization.git
make clean
make
./ex3.out conf.txt
```
