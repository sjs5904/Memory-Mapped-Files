# Memory-Mapped-Files
메모리 매핑은 여러 프로세스들이 자료 공유를 위해 사용할 수 있습니다.  
한 프로세스가 공유중인 메모리 매핑에 쓰기를 하면 그 쓰기는 즉시 다른 모든 프로세스들도 볼 수 있게됩니다.  

이 프로젝트는 3x2 차트를 여러 다른 프로세스에서 동시에 수정하고 읽는 테스트를 진행합니다.  


=====================================================================  
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

=====================================================================  
