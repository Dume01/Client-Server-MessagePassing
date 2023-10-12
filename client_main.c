#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

struct my_msgbuf
{
    long mtype;
    int service_option;
    char mtext[1000];
    short cid;
};

void recieveMsg(int msqid, struct my_msgbuf bufx)
{

    if (msgrcv(msqid, &bufx, sizeof(bufx) - sizeof(long), 1, 0) == -1)
    {
        perror("Error in msgrcv\n");
        exit(1);
    }
    if (bufx.service_option == 1){
    	printf("Recieved message from server: %s\n", bufx.mtext);  
        printf("\n"); 
    }
             
    if (bufx.service_option == 2){
    	printf("Recieved message from server: %s\n", bufx.mtext);
        printf("\n");
    }
         
    if (bufx.service_option == 3)
    {
        if(bufx.mtext[0] == '+' && strlen(bufx.mtext)==1){
        	printf("The specified file doesn't exist.\n");
        	printf("\n");
        }
        else{
        	int len = strlen(bufx.mtext);
		int cnt = 0;
		int flag=0;
		for (int i = 0; i < len; i++)
		{
		    if (bufx.mtext[i] == ' ' || bufx.mtext[i] == '\n')
		        cnt++;
                
		    if(bufx.mtext[i] == '\n'){
		    	flag=1;
		    }
		        
		}
		if(strlen(bufx.mtext)==5 && (strcmp(bufx.mtext,"Hello")==0)){
			printf("Word count for the specified file: 0\n");
			printf("\n");
		}		
		else{
			if(!flag) cnt++;
			
			printf("Word count for the specified file: %d\n", cnt);
			printf("\n");
		}
		
        }
        
    }
    
}

int main(void)
{
    // struct my_msgbuf buf;
    int msqid;
    int client_id;
    int option;
    key_t key; // getting msg queue id

    if ((key = ftok("server_main.c", 'A')) == -1)
    { //'A' project identifier
        perror("Error in ftok\n");
        exit(1);
    }

    if ((msqid = msgget(key, 0666)) == -1)
    {
        perror("Error in msgget\n");
        exit(2);
    }

    printf("Enter the client id: ");
    int flag = 1;

    while (flag)
    {
        scanf("%d", &client_id);
        if (client_id > 0)
            flag = 0;
        else
            printf("Enter valid client id\n");
    }
    printf("\n");
    while (1)
    {
        printf("Press 1 to contact the ping server\nPress 2 to contact the File Search server \nPress 3 to contact the File Word Count server\nPress 4 to Terminate\n");
        int flag = 1;

        while (flag)
        {
            scanf("%d", &option);
            if (option == 1 || option == 2 || option == 3 || option == 4)
                flag = 0;
            else
                printf("Enter valid option\n");
        }
        switch (option)
        {
        case 1:
            struct my_msgbuf buf1;
            buf1.mtype = 1;
            buf1.cid=client_id;
            buf1.service_option = option;
            char myString[] = "Hi";
            strcpy(buf1.mtext, myString);
            if (msgsnd(msqid, &buf1, sizeof(buf1) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
            printf("Sent message to server: %s\n", buf1.mtext);
            recieveMsg(msqid, buf1);
            break;
        case 2:
            struct my_msgbuf buf2;
            buf2.mtype = 1;
            buf2.cid=client_id;
            buf2.service_option = option;
            printf("Enter the filename: ");
            scanf("%s", buf2.mtext); // Remove & before buf2.mtext
           
            if (msgsnd(msqid, &buf2, sizeof(buf2) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
            recieveMsg(msqid, buf2);
            break;
        case 3:
            struct my_msgbuf buf;
            buf.mtype = 1;
            buf.cid=client_id;
            buf.service_option = option;
            printf("Enter the filename: ");
            scanf("%s", buf.mtext);            
            if (msgsnd(msqid, &buf, sizeof(buf) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
            recieveMsg(msqid, buf);
            break;
        case 4:
            while (wait(NULL) > 0)
            {
            }
            printf("Client terminating gracefully...\n");
            exit(0);
        }
    }
    return 0;
}
