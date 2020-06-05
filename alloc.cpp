#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/resource.h>
#include <sys/wait.h>

union semun {
	int val; /* used for SETVAL only */
	struct semid_ds *buf; /* for IPC_STAT and IPC_SET */
	ushort *array;
}; 

int main() {
	struct stat mystat;
	char *pmap;
	char stringOut[1];
	size_t filesize;
	int fd,userValue1,userValue2,j,avail;
	int value=0;
	int sem;
	char *p, s[100];

	key_t key;
	union semun arg;
	struct sembuf sb = {0, -1, 0};
	if ((key = ftok("res.txt", 1)) == -1) {
		perror("ftok");
		exit(1);
	}
	// create a new semaphore
	if ((sem = semget(key, 1, 0666 | IPC_CREAT)) == -1) {
		perror("semget");
		exit(1);
	} 
	// use sem = semget(key, 1, 0); to attach to an existing semaphore
	// flags also contain access rights, to take care to set them appropriately
	
	/* initialize semaphore #0 to 1: */
	arg.val = 1;
	if (semctl(sem, 0, SETVAL, arg) == -1) {
		perror("semctl");
		exit(1);
	} 
	
	// opens res.txt
	fd = open("res.txt", O_RDWR | O_CREAT, (mode_t)0600);
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
	pmap = static_cast<char*>(mmap(0, filesize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
	if(pmap==MAP_FAILED){
		perror("mmap");
		close(fd);
		exit(1);
	}
	
	printf("Mapped at %p\n", pmap);
	
	// format for the res.txt follows the format shown in the pdf file. For example,
	// 0	4
	// 1	5
	// 2	7
	// In a loop, it keeps asking how many units of a resource type is needed.
	while(1) {
		printf("PARENT: What is the resource number to be remapped?\n");
		while (fgets(s, sizeof(s), stdin)) {
				userValue1 = strtol(s, &p, 10);
				if (p == s || *p != '\n') {
					printf("Please enter an integer: ");
				} else break;
			}
		//printf("%hu %d %d\n",sb.sem_num,sb.sem_op,sb.sem_flg);
		sb.sem_op = -1; // mutual exclusion after giving first input to edit resource
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
			printf("Current value is: %d\n",value);
			if (value==userValue1){ // resource found
				printf("PARENT: Found the resource.\n");
				avail=strtol((char *)(pmap+j+2),NULL,10);
				printf("PARENT: The number of available resources for this type is %d\n",avail);
				if(avail>=userValue2){ // has enough resource
					printf("PARENT: Successfully remapped the number of resources to %d\n",avail-userValue2);
					sprintf(stringOut,"%d",(avail-userValue2));
					printf("PARENT: String = %s\n", stringOut); // update to memory region
					strncpy((char *) pmap+j+2,stringOut,1);
					//lseek(fd, 0, SEEK_SET);
					//write(fd, pmap, filesize);
					break;
				}
						
				else{
					printf("PARENT: Sorry, there are not enough resources to fulfill your request.\n");
					break;
				}
			}
			j=j+5; //Plus 5 gets us to the next line.
			if(((int)(*(char *)(pmap+j)))==0){
				printf("PARENT: Could not find the resource.\n");
			}
		}
		// write the content of the mapped file on the physical file
		msync(pmap,mystat.st_size,fd);
		sb.sem_op = 1; /* free resource */
		if (semop(sem, &sb, 1) == -1) {
			perror("semop");
			exit(1);
		} 
	}
	munmap(pmap, filesize);
	return 0;
	  
}