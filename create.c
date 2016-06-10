/*
 * create.c
 *
 *  Created on: Jun 3, 2016
 *      Author: krishna
 */

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <malloc.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>

#define RN 1 //Running Thread state
#define RB 2 //Runnable Thread State
#define FIBER_STACK 1024*64
#define EXIT 1		//Exit flag status
#define PROGRESS 0	//Exit flag status

int tidn =0;	   //Thread id initial value
int start= 0;      //Flag to indicate whether to run init function or not

typedef struct tcb tcb;
struct tcb{
	int tid; //Thread ID
	int status;// Status of thread ex Running runnable
	ucontext_t context;// store the SP PC
	int exit_flag;
	tcb *next;
};
tcb *front;
tcb *rear;
tcb *exit_node;

void enqueue (tcb *node){
	node->next = NULL;
	if (front == NULL && rear ==NULL){
		front = node;
		rear  = node;
		return;
	}
	rear->next = node;
	rear=node;

}

 tcb* dequeue (){
	if (front == NULL ){
		printf("CANNOT dequeue and EMPTY QUEUE \n");
		return NULL;
	}
	if(front == rear ){
		front=rear=NULL;
		return front;
	}else{
	front = front->next;
	return front;
	}
}

 tcb * find_parent_node (){

	 tcb *temp = front;
	  	if (temp==NULL){

	  		perror("NO Queue header \n");
	  	}

	  	while(temp!=NULL){
	  		if (temp->tid	== 1){
	  			return temp;
	  		}
	  		temp =temp->next;
	  	}

	  	return temp;

 }


 tcb *find_running_node (){


 	tcb *temp = front;
 	if (temp==NULL){

 		perror("NO Queue header \n");
 	}

 	while(temp!=NULL){
 		if (temp->status== RN){
 			return temp;
 		}
 		temp =temp->next;
 	}

 	return temp;
 }


void scheduler (){
	 tcb *running_node = find_running_node();
	 if (running_node == NULL){
	 	    		perror("NO running thread \n");
	 	    		exit(1);
	 	  }
	 running_node->status= RB;
	 tcb *next_runnable_node =dequeue();
	 while (next_runnable_node->exit_flag== EXIT){
		 /*free( &next_runnable_node->context.uc_stack.ss_sp );*/
		 free (next_runnable_node);
		 next_runnable_node =dequeue();
	 }
	 next_runnable_node->status=RN;
	 enqueue(next_runnable_node);
	 ualarm(5000,0);
	 swapcontext(&running_node->context,&next_runnable_node->context);

}


void init(){


	signal(SIGALRM,&scheduler);
	tcb *parent_node = malloc(sizeof(tcb)); 	//Allocating memory for Parent node
	parent_node->tid = ++tidn;			   		// Allocating 1 for parent node and incrementing global variable
	parent_node->status= RN;				   // Assigning running status
	parent_node->exit_flag=PROGRESS;
	parent_node->next = NULL;
	start=1;
	enqueue(parent_node);
}


void exit_function (){

	printf("Exitting and clean up \n");
	tcb *running_exit_node = find_running_node();
	running_exit_node->exit_flag=EXIT;
	makecontext( &exit_node->context, &exit_function, 0 );

}


void create_exit_context(){

	exit_node = malloc(sizeof(tcb));
	getcontext(&exit_node->context);
	tcb *parent_node= find_parent_node();
	exit_node->context.uc_link = &parent_node->context;		//Exit Context
	exit_node->context.uc_stack.ss_sp = malloc(FIBER_STACK);
	exit_node->context.uc_stack.ss_size = FIBER_STACK;
	exit_node->context.uc_stack.ss_flags = 0;
		    if ( exit_node->context.uc_stack.ss_sp == 0 )
		            {
		                   perror( "Malloc: Could not allocate stack" );
		                   exit( 1 );
		            }
    printf( "Creating child Thread\n" );
    makecontext( &exit_node->context, &exit_function, 0 );
}




void *thread_function1(void *args){

	int *value = (int *)args;
	int v= *value;
	printf("New Thread created & value sent while creating = %d \n",v);
	while(1){

		printf("thread1 \n");

	}
	return NULL;
}


void *thread_function2(void *args){

	int *value = (int *)args;
	int v= *value;
	printf("New Thread created & value sent while creating = %d \n",v);
	while(1){

		printf("thread2 \n");

	}
	return NULL;
}





void thread_create(int *tid, void *ptr, void *(*func) (void *) ,void *args){
	if (start == 0){
		printf("First thread Creation \n Initializing parent Node \n");
		init();
		create_exit_context();
	}
	tcb *new_node = malloc(sizeof(tcb)); 		//Allocating memory for Parent node
	int tidno =tidn++;
	new_node->tid = tidno;			   			// Allocating 1 for parent node and incrementing global variable
	new_node->status= RN;				 	    // Assigning running status
	new_node->exit_flag=PROGRESS;

    getcontext(&new_node->context);
    new_node->context.uc_link = &exit_node->context;		//Exit Context
    new_node->context.uc_stack.ss_sp = malloc( FIBER_STACK );
    new_node->context.uc_stack.ss_size = FIBER_STACK;
    new_node->context.uc_stack.ss_flags = 0;
    if ( new_node->context.uc_stack.ss_sp == 0 )
            {
                   perror( "Malloc: Could not allocate stack" );
                   exit( 1 );
            }
    printf( "Creating child Thread\n" );
    makecontext( &new_node->context, (void (*)(void))func, 1, args );

    tcb *running_node = find_running_node();
    if (running_node == NULL)
    	{
    		perror("NO running thread \n");
    		exit(1);
    	}
    running_node->status =RB;
    enqueue(new_node);
    printf( " Swapping to  new context\n" );
    ualarm(5000,0);
    swapcontext(&running_node->context,&new_node->context);

}






int main()
{
		int i= 100;
		int j= 200;
		int k= 300;
		thread_create(NULL, NULL,&thread_function1,&i );
		thread_create(NULL, NULL,&thread_function2,&j );
       return 0;
}

