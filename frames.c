#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define mxn 1048576

struct node
{
    struct node *last;
    struct node *next;
    int val;
}*arr[mxn];

char *stype;
int pagetable[mxn];
int invtable[1000];
int secondchance[1000];
int last_access[1000];
char presentBit[mxn];
char dirtyBit[mxn];
int frames = 0;
int verbose = 0;
int n = 1000;
int starting_index = 0;
int clock_start = 0;

int no_mem_access = 0;
int writes = 0;
int drops = 0;
int misses = 0;

void type_1(char *sfile, int n);
void type_2(char *sfile, int n);
void fifo(int vpn, char mode_bit);
void rndm(int vpn, char mode_bit);
void clck(int vpn, char mode_bit);
void lru(int vpn, char mode_bit, int time);
void opt(int vpn, char mode_bit, int time);



int main(int argc, char *argv[]){
	srand(5635);
	char *sfile;
	verbose = 0;
	if(argc < 4){
		printf("The parameters supplied are too few \n");
	}
	else if(argc == 4){
		n = atoi(argv[2]);
		sfile = argv[1];
		stype = argv[3];
	}
	else if(argc >= 5){
		n = atoi(argv[2]);
		sfile = argv[1];
		stype = argv[3];
		if(strcmp(argv[4], "-verbose") == 0){
			verbose = 1;
		}
		else{
			printf("Invalid parameters \n");	
		}
	}
	else{
		printf("Invalid parameters \n");
	}

	if(strcmp(stype, "OPT") == 0){
		type_2(sfile, n);
	}
	else if(strcmp(stype, "FIFO") == 0 || strcmp(stype, "CLOCK") == 0 || strcmp(stype, "LRU") == 0 || strcmp(stype, "RANDOM") == 0){
		type_1(sfile, n);
	}
	else{
		printf("Wrong code for type of technique to use \n");
	}
	printf("Number of memory accesses: %d\n", no_mem_access);
	printf("Number of misses: %d\n", misses);
	printf("Number of writes: %d\n", writes);
	printf("Number of drops: %d\n", drops);
    return 0;

}

void type_1(char *sfile, int n){
	FILE * fp = fopen(sfile, "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if (fp == NULL){
        exit(EXIT_FAILURE);
    }
    //initializing
    for (int i = 0; i < mxn; ++i)
    {
    	pagetable[i] = -1;
		presentBit[i] = 'F';
		dirtyBit[i] = 'F';
    }
    for (int i = 0; i < n; ++i)
    {
    	invtable[i] = -1;
    	last_access[i] = -1;
    	secondchance[i] = 0;
    }
    int loop_counter = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        // tokenizing the input to separate the page_code from the accessmode
        char **command = malloc(4 * sizeof(char *));
	    if (command == NULL) {
	        perror("malloc failed");
	        exit(1);
	    }
	    char *separator = " \t\n";
	    char *parsed;
	    int index = 0;
	    parsed = strtok(line, separator);
	    while (parsed != NULL) {
	        command[index] = parsed;
	        index++;
	        parsed = strtok(NULL, separator);   
	    }
	    command[index] = NULL;
	    char *curr_page = command[0]; //hexa
	    char access_bit = *command[1];

	    long long_add = strtol(curr_page, NULL, 0);
	    int vpn = (int) (long_add >> 12);

	    if(strcmp(stype, "FIFO") == 0){
			fifo(vpn, access_bit);
		}
		else if(strcmp(stype, "CLOCK") == 0){
			clck(vpn, access_bit);
		}
		else if(strcmp(stype, "LRU") == 0){
			lru(vpn, access_bit, loop_counter);
		}
		else if(strcmp(stype, "RANDOM") == 0){
			rndm(vpn, access_bit);
		}
		else if(strcmp(stype, "OPT") == 0){
			opt(vpn, access_bit, loop_counter);
		}

		else{
			printf("Wrong code for type of technique to use \n");
		}
		no_mem_access++;
		loop_counter++;
		// printf("%d\n", n);
	    free(command);
    }

    fclose(fp);
    if (line){
    	free(line);
    }
    return;
}

void type_2(char *sfile, int n){
	FILE * fp = fopen(sfile, "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    if (fp == NULL){
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < mxn; ++i)
    {
    	arr[i] = NULL;
    }
    int loop_counter = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        // tokenizing the input to separate the page_code from the accessmode
        char **command = malloc(4 * sizeof(char *));
	    if (command == NULL) {
	        perror("malloc failed");
	        exit(1);
	    }
	    char *separator = " \t\n";
	    char *parsed;
	    int index = 0;
	    parsed = strtok(line, separator);
	    while (parsed != NULL) {
	        command[index] = parsed;
	        index++;
	        parsed = strtok(NULL, separator);   
	    }
	    command[index] = NULL;
	    char *curr_page = command[0]; //hexa
	    char access_bit = *command[1];

	    long long_add = strtol(curr_page, NULL, 0);
	    int vpn = (int) (long_add >> 12);

	    // here we make the occurances 2d-array
	    struct node *temp;
	   	if(arr[vpn] == NULL){
	   		arr[vpn] = (struct node *) malloc(sizeof(struct node));
	   		arr[vpn]->next = NULL;
	   		arr[vpn]->last = arr[vpn];
	   		arr[vpn]->val = loop_counter;
	   	}
	   	else{
	   		temp = arr[vpn]->last;
	   		temp->next = (struct node *) malloc(sizeof(struct node));
	   		temp = temp->next;
	   		temp->next = NULL;
	   		temp->last = NULL;
	   		temp->val = loop_counter;
	   		arr[vpn]->last = temp;
	   	}

		loop_counter++;
	    free(command);
    }
    fclose(fp);
    if (line){
    	free(line);
    }
    type_1(sfile, n);
    return;
}



void fifo(int vpn, char mode_bit)
{
	char curr_present_bit = presentBit[vpn];
	if(curr_present_bit == 'T'){
		if(mode_bit == 'W'){
			dirtyBit[vpn] = 'T';
		}
	}
	else{
		misses++;
		// here we add this page to the physical memory
		presentBit[vpn] = 'T';
		if(frames < n){
			invtable[frames] = vpn;
			pagetable[vpn] = frames;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			frames++;
		}
		else{
			int removedPage = invtable[starting_index];
			presentBit[removedPage] = 'F';
			if(dirtyBit[removedPage] == 'T'){
				writes++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was written to the disk.\n", vpn, removedPage);		
				}
			}
			else{
				drops++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n", vpn, removedPage);		
				}
			}
			invtable[starting_index] = vpn;
			pagetable[vpn] = starting_index;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			starting_index = (starting_index + 1)%n;
		}

	}
}

void rndm(int vpn, char mode_bit)
{
	char curr_present_bit = presentBit[vpn];
	if(curr_present_bit == 'T'){
		if(mode_bit == 'W'){
			dirtyBit[vpn] = 'T';
		}
	}
	else{
		misses++;
		// here we add this page to the physical memory
		presentBit[vpn] = 'T';
		if(frames < n){
			invtable[frames] = vpn;
			pagetable[vpn] = frames;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			frames++;
		}
		else{
			int rand_index = rand()%n;
			int removedPage = invtable[rand_index];
			presentBit[removedPage] = 'F';
			if(dirtyBit[removedPage] == 'T'){
				writes++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was written to the disk.\n", vpn, removedPage);		
				}
			}
			else{
				drops++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n", vpn, removedPage);		
				}
			}
			invtable[rand_index] = vpn;
			pagetable[vpn] = rand_index;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
		}

	}
}

void lru(int vpn, char mode_bit, int time)
{
	char curr_present_bit = presentBit[vpn];
	if(curr_present_bit == 'T'){
		last_access[pagetable[vpn]] = time;
		if(mode_bit == 'W'){
			dirtyBit[vpn] = 'T';
		}
	}
	else{
		misses++;
		// here we add this page to the physical memory
		presentBit[vpn] = 'T';
		if(frames < n){
			invtable[frames] = vpn;
			pagetable[vpn] = frames;
			last_access[frames] = time;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			frames++;
		}
		else{
			int rm_index = 0;
			int mn_val = last_access[0];
			for (int i = 0; i < n; ++i)
			{
				if(mn_val > last_access[i]){
					mn_val = last_access[i];
					rm_index = i;
				}
			}
			int removedPage = invtable[rm_index];
			presentBit[removedPage] = 'F';
			if(dirtyBit[removedPage] == 'T'){
				writes++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was written to the disk.\n", vpn, removedPage);		
				}
			}
			else{
				drops++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n", vpn, removedPage);		
				}
			}
			invtable[rm_index] = vpn;
			pagetable[vpn] = rm_index;
			last_access[rm_index] = time;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
		}

	}
}

void opt(int vpn, char mode_bit, int loop_counter)
{
	struct node *temp = arr[vpn]->next;
	struct node *sheep = arr[vpn]->last;
	if(temp != NULL){
		temp->last = sheep;
		arr[vpn] = temp;
	}
	else{
		arr[vpn] = NULL;
	}

	char curr_present_bit = presentBit[vpn];
	if(curr_present_bit == 'T'){
		if(mode_bit == 'W'){
			dirtyBit[vpn] = 'T';
		}
	}
	else{
		misses++;
		// here we add this page to the physical memory
		presentBit[vpn] = 'T';
		if(frames < n){
			invtable[frames] = vpn;
			pagetable[vpn] = frames;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			frames++;
		}
		else{
			int rm_index = 0;
			int value = -1;
			for (int i = 0; i < n; ++i)
			{
				if(arr[invtable[i]] != NULL){
					if(value < arr[invtable[i]]->val){
						value = arr[invtable[i]]->val;
						rm_index = i;
					}

				}
				else{
					rm_index = i;
					break;
				}
			}
			int removedPage = invtable[rm_index];
			presentBit[removedPage] = 'F';
			if(dirtyBit[removedPage] == 'T'){
				writes++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was written to the disk.\n", vpn, removedPage);		
				}
			}
			else{
				drops++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n", vpn, removedPage);		
				}
			}
			invtable[rm_index] = vpn;
			pagetable[vpn] = rm_index;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}

		}

	}
}

void clck(int vpn, char mode_bit)
{
	char curr_present_bit = presentBit[vpn];
	if(curr_present_bit == 'T'){
		if(mode_bit == 'W'){
			dirtyBit[vpn] = 'T';
		}
		secondchance[pagetable[vpn]] = 1;
	}
	else{
		misses++;
		// here we add this page to the physical memory
		presentBit[vpn] = 'T';
		if(frames < n){
			invtable[frames] = vpn;
			pagetable[vpn] = frames;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			secondchance[frames] = 0;
			frames++;
		}
		else{
			int rm_index = clock_start;
			
			while(secondchance[rm_index] == 1){
				secondchance[rm_index]--;
				rm_index = (rm_index + 1)%n;
			}

			clock_start = rm_index;
			int removedPage = invtable[clock_start];
			presentBit[removedPage] = 'F';
			if(dirtyBit[removedPage] == 'T'){
				writes++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was written to the disk.\n", vpn, removedPage);		
				}
			}
			else{
				drops++;
				if(verbose == 1){
					printf("Page %#07x was read from disk, page %#07x was dropped (it was not dirty).\n", vpn, removedPage);		
				}
			}
			invtable[clock_start] = vpn;
			pagetable[vpn] = clock_start;
			secondchance[clock_start] = 1;
			if(mode_bit == 'R'){
				dirtyBit[vpn] = 'F';	
			}
			else{
				dirtyBit[vpn] = 'T';
			}
			clock_start = (clock_start + 1)%n;
		}

	}
}
