# OSProject
We'll be documenting our Operating Systems end-of semester project journey. Our project will be focused on deadlocks and how different algorithms work to solve and prevent deadlocks.
We will be simulating a banking system with some accounts but immeasurably more transactions occurring between said accounts. In a real-world banking environment, thousands of transactions firing simultaneously means threads will inevitably collide — racing to acquire the same locks, stalling each other, and risking deadlock if not carefully managed. We have incorporated 3 such algorithms — Banker's algorithm, Wait-Die, and the Resource Allocation Graph — to help reduce the likelihood of deadlocks.
Each algo has its own mechanism of detecting and solving the deadlock, as shown below:
