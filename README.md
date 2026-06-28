# OSProject
We'll be documenting our Operating Systems end-of semester project journey. Our project will be focused on deadlocks and how different algorithms work to solve and prevent deadlocks.

We will be simulating a banking system with some accounts but immeasurably more transactions occurring between said accounts. In a real-world banking environment, thousands of transactions firing simultaneously means threads will inevitably collide — racing to acquire the same locks, stalling each other, and risking deadlock if not carefully managed. 
We have incorporated 3 such algorithms — Banker's algorithm, Wait-Die, and the Resource Allocation Graph — to help reduce the likelihood of deadlocks.

We used general functions: algo_init, algo_request, algo_release, algo_print_state in the main.c to allow seamless integration of any algorithm at runtime. 
These have been declared in the deadlock_algo.h that is included by all the algorithms. Thus allowing the algorithms to define their own way of running the function. 

The makefile compiles the selected algorithm along with the main to run the necessary commands and display the output.

Each algo has its own mechanism of detecting and solving the deadlock, as shown below:

Banker's Algorithm:

https://github.com/user-attachments/assets/d4742e63-f1dc-4a47-bd3d-a64ea6a0e5c0

Wait_die:

https://github.com/user-attachments/assets/dd57b44a-b691-42e9-9eca-bedf1b3f9340

Resource Allocation Graph:

https://github.com/user-attachments/assets/48053a69-b1cc-433f-913f-3c16c6aab471
