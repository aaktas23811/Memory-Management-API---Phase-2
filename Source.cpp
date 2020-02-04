#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <stdlib.h> 
#include <queue> 
#include <semaphore.h>
using namespace std;



#define NUM_THREADS 3
#define MEMORY_SIZE 10
char memarray[MEMORY_SIZE];

struct node
{
	int id;
	int size;
	int index;
	node *next;
	node()
		:next(NULL)
	{}
	node(int i, int sizePar, int indexPar, node *p = NULL)
		: id(i), size(sizePar), index(indexPar), next(p) {};

};
class Linkedlist
{
public:
	Linkedlist();
	bool allocation(int id, int requestsize);
	node* headpoint();
	private:
	node *head;
};
node* Linkedlist :: headpoint()
{
	return head;
}
Linkedlist:: Linkedlist()
{
	head = new node(-1,MEMORY_SIZE, 0);
}
bool Linkedlist:: allocation(int id, int requestsize)
{
	node *temp = head;
	while (temp != NULL)
	{
		if (temp->id == -1 && temp->size >= requestsize)
		{
			temp->id = id;
			if ((temp->size - requestsize) != 0 )// the case when request size smaller than hole  size
			{
				
				node * temporal = new node(-1, temp->size - requestsize, temp->index + requestsize, temp->next);
				temp->next = temporal;
				temp->size=requestsize;
			}
			else
				temp->size=requestsize;
			for (int i = temp->index; i < (temp->index+requestsize); i++)
			{
				char c = '0' + id;
				memarray[i] = c;
			}
			return true;
		}
		temp = temp->next;
	}
	return false;

}

struct reqnode
{
	int id;
	int size;
	reqnode(int i, int s) : id(i), size(s)
	{};
};

queue<reqnode> memoryqueue; // shared que
Linkedlist myList;
pthread_mutex_t sharedLock = PTHREAD_MUTEX_INITIALIZER; // mutex
pthread_t serverthread; // server thread handle
sem_t semlist[NUM_THREADS]; // thread semaphores
int thread_message[NUM_THREADS]; // thread memory information
bool terminating = true; 

void release_function()
{
	terminating = false;
	node* temp =myList.headpoint();
	node* p=temp;
	while(temp != NULL)
	{
		p=temp->next;
		delete temp;
		temp=p;
	}
	temp=NULL;////////////////////////////	
	for (int i = 0; i < MEMORY_SIZE; i++)//retuned initial state
	{		
		memarray[i] = 'X';
	}
}


void my_malloc(int thid, int size) // requested size added to shared global queue
{
	pthread_mutex_lock(&sharedLock); //lock
	reqnode request(thid, size); // new queue node created and pushed the global queue
	memoryqueue.push(request);
	pthread_mutex_unlock(&sharedLock); //unlock								   
}

void use_mem() // sleeps the thread between 1-5 seconds
{
	int random = (rand() % 5) + 1;
	sleep(random);
}

void free_mem(int Id) // frees the memory
{

//	List.freeMemory(ID);
	node* temp= myList.headpoint();
	node * prev = temp;
	while(temp != NULL && temp->id != Id)//iteration until the given id found
	{
		prev=temp;
		temp=temp->next;
	}
		if(temp==NULL) //if the list is empty
		{
			return;
		}

	if(temp== myList.headpoint() && temp->next == NULL)//there is only one node
	{
		temp->id=-1; // becomes hole
		temp->size= MEMORY_SIZE;
		temp->index=0;
		for (int i = 0; i < MEMORY_SIZE; i++)
		{
			char X = 'X';
			memarray[i] = X;
		}	
	}
	else if (temp== myList.headpoint())
	{
		if(temp->next->id != -1)
		{
			temp->id=-1;
			for (int i = temp->index; i < temp->size; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}
		}
		else
		{
			node* p=temp->next;
			temp->id=-1;
			temp->size=temp->size+ p->size;
			temp->next= p->next;
			delete p;
			p=NULL;
			for (int i = temp->index; i < temp->size; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}
		}
	}
	else if(temp->next == NULL)
	{
		if(prev->id != -1 )
		{
			temp->id=-1;
			for (int i = temp->index; i < temp->size; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}

		}
		else
		{
			prev->next=temp->next;
			prev->size =prev->size+temp->size;
			delete temp;
			temp=NULL;
				for (int i = prev->index; i < prev->size; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}
		}
	}
	else if (temp->next != NULL  && prev != temp)// case that in the middle
	{
		if (prev->id != -1 && temp->next->id != -1)
		{
			for (int i = temp->index; i < temp->size+ temp->index; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}
				temp->id= -1;
		}
		else if (temp->next->id == -1 && prev->id==-1)
		{
			node* p= temp->next;
			prev->size +=temp->size;
			prev->size+=p->size;
			prev->id= -1;
			prev->next=temp->next->next;
			node* q=temp;
			delete q;
			delete p;
			p=NULL;
			q=NULL;
			for (int i = prev->index; i <prev->size + prev->index ; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}

		}
		else if(prev->id != -1 && temp->next->id ==-1)
		{
			node* p=temp->next;
			temp->id= -1;
			temp->size +=temp->next->size;
			temp->next=p->next;
			delete p;
			p=NULL;
				for (int i = temp->index; i < temp->size + temp->index; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}
		}
		else if (temp->next->id != -1 && prev->id == -1)
		{
			prev->next=temp->next;
			prev->size= prev->size+ temp->size;
			prev->id== -1;
			delete temp;
			temp= NULL;
				for (int i = prev->index; i < prev->size+prev->index; i++)
			{
				char X = 'X';
				memarray[i] = X;
			}
		}
	}


	pthread_mutex_unlock(&sharedLock);
}

void dump_memory() // print the content
{
	cout<< "List : \n";
	node* p= myList.headpoint();
	while(p!= NULL)
	{
		cout << "[" << p->id << "][" << p->size << "][" << p->index << "]";
		if (p->next != NULL)
			cout << "---";
		p =p->next;
	}


	cout << "\nMemory Dump:" << endl;
	for (int i = 0; i < MEMORY_SIZE; i++)
		cout << memarray[i];
	cout << "\n\n*********************************\n";
}

void* server_function (void*)
{
	while(terminating)
	{

		if(!memoryqueue.empty() && terminating)
		{
			pthread_mutex_lock(&sharedLock); //lock
			reqnode request= memoryqueue.front();
			int size=request.size;
			int id=request.id;
			if(myList.allocation(id,size))
			{
				dump_memory();
				thread_message[id]= 1;
			}
			else
			{
				thread_message[id]= -1;
			}
			memoryqueue.pop();
			sem_post(&(semlist[id]));
			pthread_mutex_unlock(&sharedLock); //unlock
		}


	}
}

void * thread_function(void * id)
{
	while(terminating){
	int ransize=rand() % (MEMORY_SIZE / 3) +1;
	int *idT = (int*)id;
	int idthread= *idT;
	my_malloc(idthread, ransize);
	sem_wait(&semlist[idthread]);
	if (thread_message[idthread] == 1)
	{
			use_mem();
		pthread_mutex_lock(&sharedLock);
		free_mem(idthread);
		
		pthread_mutex_unlock(&sharedLock);
		
		}
	}
}


int main (int argc, char *argv[])
{
	for (int i = 0; i < NUM_THREADS; i++) //initialize semaphores
	{
		sem_init(&semlist[i], 0, 0);
	}

	for (int i = 0; i < MEMORY_SIZE; i++)	//initialize memory 
	{
		char X = 'X'; memarray[i] = X;
	}

	pthread_create(&serverthread, NULL, server_function, NULL); //start server 

	int ids[NUM_THREADS];
	for (int i =0 ;i<NUM_THREADS; i++ )
		ids[i]=i;


	pthread_t threads[NUM_THREADS];
	for (int i = 0; i < NUM_THREADS; i++)
		pthread_create(&threads[i], NULL, &thread_function, (void*)&ids[i]);

	sleep(10);
	terminating=false;
	release_function();

	pthread_join(serverthread, NULL);
	for (int k = 0; k < NUM_THREADS; k++)
		pthread_join(threads[k], NULL);

	
	printf("\nTerminating...\n");

}
