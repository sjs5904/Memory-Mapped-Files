COM S 352
Project # 2
Jooseung Song
Memory Mapped Files
shear.cpp
prov-rep.cpp
res.txt

alloc.cpp
this reads 3x2 chart holding set of available resource
It asks user how many units of a resource type is being used.
Prints result, do synchronous write and ask again
Initialize new semaphore. 

prov-rep.cpp
grabs semaphore that was initialized at alloc.cpp
reads 3x2 chart holding set of available resource
generates child process
	Parent:
	It asks user how many units of a resource type is being added.
	Prints result, do synchronous write and ask again
	Child:
	Reports three things every 10 seconds.
	This process is undisturbed.


All three system calls written in the project description are 
used: mmap(), fstat(), msync(), getpagesize(), mincore(), semget(), semctl(), and semop().

Mutual Exclusion
for each different execution,
alloc and prov-rep cannot enter resource editing section at the same time.
Once user enters the first input integer, the other process is blocked calling semop().

Makefile
-"make" or "make all" command will make file 'alloc' and 'prov-rep'
	command line1: make all
	command line1: ./alloc
	
	command line2: ./prov-rep

res.txt
-text file with set of available resource

15/03/2019