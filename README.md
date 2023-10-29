# operating_ex3

## Introduction
The purpose of this project is to gain experience with concurrent programming and synchronization mechanisms.
The overall scenario we are simulating is that of news broadcasting. Different types of stories are produced and the system sorts them and displays them to the public.

## Table of contents
* [General Information](#general-information)
* [Installation](#installation)

## General Information
In this assignment, the ‘new stories’ are simulated by simple strings which should be displayed on the screen in the order they arrive.
In the scenario, there are 4 types of active actors: 

- Producer
Each producer creates some strings in the following format: "producer i type j"  where ‘i’ is the producer's ID, ‘type' is a random type it chooses which can be ‘SPORTS’, ‘NEWS’, 'WEATHER’, and ‘j’ is the number of strings of type ‘type’ this producer has already produced. Each of the producers passes its information to the Dispatcher via its private queue. Each of the Producer's private queue is shared between the Producer and the Dispatcher. Each of the string products is inserted by the Producer into its ‘producers queue’. After inserting all the products, the Producer sends a ‘DONE’ string through its Producer queue.

For example,
![image](https://github.com/adi-ben-yehuda/operating_ex3/assets/75027826/4d29c789-a607-4413-8f0c-9e759a171307)

### Dispatcher
The Dispatcher continuously accepts messages from the Producers' queues. It scans the Producer's queue using a Round Robin algorithm. The Dispatcher does not block when the queues are empty. Each message is "sorted" by the Dispatcher and inserted into one of the Dispatcher queues which includes strings of a single type. When the Dispatcher receives a "DONE" message from all Producers, it sends a "DONE" message through each of its queues.

### Co-Editors
For each type of possible message there is a Co-Editor that receives the message through the Dispatchers queue, "edits" it, and passes it to the screen manager via a single shared queue. The editing process will be simulated by the Co-Editors by blocking for one-tenth (0.1) of a second. When a Co-Editor receives a "DONE" message, it passes it without waiting through the shared queue.

### Screen-manager
The Screen-manager displays the strings it receives via the Co-Editors queue to the screen. After printing all messages to the screen and receiving three "DONE" messages, the Screen manager displays a ‘DONE’ statement.

System Design
![image](https://github.com/adi-ben-yehuda/operating_ex3/assets/75027826/d6c552fb-242a-4bc7-adc0-5303cfd50625)


## Installation
Before installing this project, you need to install on your computer:
* Git
* Node.js
* React
* MongoDB

Then open a terminal.
write the next command:
```
npm i react-router-dom
npx create-react-app yourAppName
```
