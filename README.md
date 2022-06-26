# EDM Project

POC project in DMS(Distributed Shared Memory).

/poc directory implements the following design:

![image](https://user-images.githubusercontent.com/62066172/175827188-6db9c3e0-021e-495c-8643-a9609808b6b4.png)

Command Execution:

mpirun -n 1 manager <\start virtual address> <\num of pages>



prerequisites:
- OpenMPI 3
- Linux Kernel 5.7+
