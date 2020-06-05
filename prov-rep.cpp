#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/resource.h>
#include <sys/wait.h>

int  main(void){
	//int fd[2],fd2[2],  nbytes,nbytes2;
	pid_t childpid;
	int value=0;
	size_t filesize;
	int sem;
	char *p, s[100];
		
	key_t key;
	struct sembuf sb = {0, -1, 0};
	if ((key = ftok("res.txt", 1)) == -1) {
		perror("ftok");
		exit(1);
	}
	
	// grab semaphore
	if ((sem = semget(key, 1, 0)) == -1) {
		perror("semget");
		exit(1);
	} 
	
	int fd,j,avail;
	char stringOut[1];
	struct stat mystat;
	char *pmap;
	int userValue1,userValue2;
		
	// opens res.txt
	fd=open("res.txt", O_RDWR | O_CREAT, (mode_t)0600);
	if(fd==-1)
	{ 
		perror("open");
		exit(1);
	}
	if (fstat(fd,&mystat)<0)
	{
		perror("fstat");
		close(fd);
		exit(1);
	
	}
	
	// maps it to a memory region. Input exact file size for the region size
	filesize = mystat.st_size;
	pmap=static_cast<char*>(mmap(NULL,filesize,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0));
	if(pmap==MAP_FAILED){
		perror("mmap");
		close(fd);
		exit(1);
		
	}

	// creates a Child process
    if((childpid = fork()) ==  -1){
        perror ("fork");
        exit (0);
    }
	
	// format for the res.txt follows the format shown in the pdf file. For example,
	// 0	4
	// 1	5
	// 2	7
	if(childpid  == 0){ //Child-Reporter
        printf("CHILD:Child is alive.\n");
		int pageSize;
        size_t minAllo=getpagesize();
		printf("CHILD: minimum allocation: %d\n",minAllo);
		printf("CHILD: file size: %d\n",filesize);
		size_t tempA;
        int test=0;
		unsigned char *vector;
        while(1){
			//The page size of the system using the system call getpagesize().
			pageSize=getpagesize();
            printf("CHILD: page size is: %d\n",pageSize);
			vector=(unsigned char *) calloc(1,(filesize+pageSize-1)/pageSize);
			while((int)(*(char *)(pmap+test))!=0){ // will report 3 things since there is only 3 types
				value=strtol((char *)(pmap+test),NULL,10);
				avail=strtol((char *)(pmap+test+2),NULL,10);
				printf("CHILD: Resource: %d has %d.\n",value,avail);
				test=test+5; //to the next line.
			};
			// The current status of pages in the memory region using the system call mincore() 
			mincore(pmap,filesize,vector);
            for(tempA=0;tempA<=filesize/pageSize;tempA++){
				if(vector[tempA]&1){
					printf("CHILD: Current status of pages: %lu\n",(unsigned long int)tempA);
				}
				free(vector);
            }
            test=0;
			// count 10 seconds
            sleep(10);
        }
        exit (0);
    }

    else{ //Parent-Provider
		// In a loop, it keeps asking whether new resources need to be added.
		while(1){
			printf("PARENT: What is the resource number to be remapped?\n");
			while (fgets(s, sizeof(s), stdin)) { 
				userValue1 = strtol(s, &p, 10);
				if (p == s || *p != '\n') {
					printf("Please enter an integer: ");
				} else break;
			} // if yes. In the description, there was no case for answering "no"
			sb.sem_op = -1;
			if (semop(sem, &sb, 1) == -1) {
				perror("semop");
				exit(1);
			} 
			printf("PARENT: What is the number of resources to be remapped/added?\n");
			while (fgets(s, sizeof(s), stdin)) {
				userValue2 = strtol(s, &p, 10);
				if (p == s || *p != '\n') {
					printf("Please enter an integer: ");
				} else break;
			}
			printf("PARENT: You entered: %d %d \n",userValue1,userValue2);
			j=0;
			while(((int)(*(char *)(pmap+j)))!=0){   
				value=strtol((char *)(pmap+j),NULL,10);
				//printf("Current value is: %d\n",value);
				if (value==userValue1){ // resource found
					printf("PARENT: Found the resource.\n");
					avail=strtol((char *)(pmap+j+2),NULL,10);
					printf("PARENT: The number of available resources for this type is %d\n",avail);
					if(avail+userValue2<10){ //resource is strictly less than 10.
						printf("PARENT: Successfully remapped the number of resources to %d\n",userValue2+avail);
						sprintf(stringOut,"%d",(userValue2+avail));
						strncpy((char *) pmap+j+2,stringOut,1); // update to memory region
						break;
					}
					else{
						printf("PARENT: Sorry, the number of units of each resource is strictly less than 10.\n");
						break;
					}
				}
				
				j=j+5; //to the next line.
				if(((int)(*(char *)(pmap+j)))!=0){
					printf("PARENT: Could not find the resource.\n");
				}
			}
			msync(pmap,mystat.st_size,fd);
			sb.sem_op = 1; /* free resource */
			if (semop(sem, &sb, 1) == -1) {
				perror("semop");
				exit(1);
			} 
		}
    }
	munmap(pmap, filesize);
    return  0;
}