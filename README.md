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

### Producer
Each producer creates some strings in the following format: “producer <i>  <type>  <j>”  where ‘i’ is the producer's ID, ‘type' is a random type it chooses which can be ‘SPORTS’, ‘NEWS’, 'WEATHER’, and ‘j’ is the number of strings of type ‘type’ this producer has already produced. Each of the producers passes its information to the Dispatcher via its private queue. Each of the Producer's private queue is shared between the Producer and the Dispatcher. Each of the string products is inserted by the Producer into its ‘producers queue’. After inserting all the products, the Producer sends a ‘DONE’ string through its Producer queue.

### Dispatcher


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
