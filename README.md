# Multi-Threaded-Ticket-Seller


Implemented a C program that uses the Pthread library to create threads in order to simulate ticket sellers who sell tickets based on their category for a concert of 100 seats.

For each of the 10 sellers a customer queue is created having N customers. Customer arrival times are in terms of minutes and are randomly generated. Customers arrive at the beginning of the minute.Customers are served based on the seller type (H,M,L). Once a seller begins to serve a customer it will look for a seat in the AVAILABLE state which is denoted by a locked mutex to ensure only one seller can access a given seat at a given time. Once the seller reserves the seat the mutex is unlocked. Similarly all customers are served until 1 hour unless the seats are sold out. If some customers are not served within the 1 hour time frame, they are turned away. At the end of the simulation, the average response time, average turnaround time and average throughput is calculated for each seller type. The number of customers served and turned away are also calculated.
