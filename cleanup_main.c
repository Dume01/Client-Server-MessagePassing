#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MSG_TYPE 1

struct my_msgbuf
{
    long mtype;
    int service_option;
    char mtext[1000];
};

int main()
{
    key_t key;
    int msqid;
    struct my_msgbuf buf3;

    if ((key = ftok("server_main.c", 'A')) == -1)
    {
        perror("Error in ftok\n");
        exit(1);
    }

    if ((msqid = msgget(key, 0666 )) == -1)
    {
        perror("Error in msgget\n");
        exit(2);
    }

    while (1)
    {
        char choice;

        printf("Do you want the server to terminate? Press Y for Yes and N for No: ");
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y')
        {
            // Send termination message to server
            buf3.mtype = MSG_TYPE;
            buf3.service_option = 5;
            char tempString[] = "Termination requested.";
            strcpy(buf3.mtext, tempString);

            if (msgsnd(msqid, &buf3, sizeof(char), 0) == -1)
            {
                perror("error in msgsnd");
                exit(1);
            }

            printf("Terminating main server gracefully...\n");
            exit(0);
        }
        else if (choice == 'N' || choice == 'n')
        {
            // Continue running as usual
        }
        else
        {
            printf("Invalid choice. Please enter Y or N.\n");
        }
    }
    return 0;
}
