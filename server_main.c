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

void handleClientRequest(struct my_msgbuf msg, int msqid)
{

    int option = msg.service_option;

    switch (option)
    {
    case 1:
        
        printf("Recieved message from client: %s\n",msg.mtext);
        char myString[] = "Hello";
        strcpy(msg.mtext, myString);
        if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
        }
        printf("Sent message to client: %s\n", msg.mtext);
        printf("\n");
        break;
    case 2:

        char command[1000 + 3]; // +3 for "ls " and null terminator
        snprintf(command, sizeof(command), "ls %s", msg.mtext);
        printf("File Search requested by client.\n");
        
        FILE *ls_output = popen(command, "r");
        if (ls_output == NULL)
        {
            perror("popen");
            exit(1);
        }

        // Read the output of the "ls" command
        char result[1000];
        if (fgets(result, sizeof(result), ls_output) != NULL)
        {
            snprintf(msg.mtext, sizeof(msg.mtext), "Found");
        }
        else
        {
            snprintf(msg.mtext, sizeof(msg.mtext), "Not found");
        }

        // Close the "ls" command output
        pclose(ls_output);

        // Send the result back to the client
        if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
        {
            perror("msgsnd");
        }
        printf("\n");
        break;
    case 3:
        int fd[2];
        printf("Word Count requested by client.\n");
        if (pipe(fd) == -1)
        {
            perror("Pipe not created\n");
            exit(0);
        }

        pid_t childp = fork();
        if (childp < 0)
        {
            perror("Error in forking\n");
            exit(0);
        }

        if (childp == 0)
        {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);

            // Redirect stdout to the message buffer
            dup2(STDOUT_FILENO, fd[1]);
            close(fd[1]);

            execlp("wc", "wc", "-w", NULL);
            perror("Error in execlp\n");
            exit(0);
        }
        else
        {
            close(fd[0]);

            // Open the file for reading
            int file_fd = open(msg.mtext, O_RDONLY);
            if (file_fd == -1)
            {
                perror("Error opening file\n");
                strcpy(msg.mtext,"+");
                if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
		        {
		             perror("msgsnd");
		        }
		        printf("\n");
		        break;
            }

            // Create a separate buffer for reading from the file
            char file_buffer[1000]; // Adjust the size as needed

            ssize_t read_count;
            while ((read_count = read(file_fd, file_buffer, sizeof(file_buffer))) > 0)
            {
                // Write data from file_buffer to the pipe
                write(fd[1], file_buffer, read_count);
            }

            close(fd[1]);

            int status;
            if (waitpid(childp, &status, 0) == -1)
            {
                perror("Error in waitpid\n");
                exit(1);
            }

            if (WIFEXITED(status))
            {
                int exit_status = WEXITSTATUS(status);
                // Handle the child process exit status, if needed
                // You can use 'exit_status' to check if the child process terminated normally
            }

            // Send the response message to the client
            printf("Result sent to client.\n");
            while (read(fd[1], file_buffer, 1000) > 0)
            {
                printf("%s", file_buffer);
            }
            
            strcpy(msg.mtext, file_buffer);
            if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
            }
        }
        printf("\n");
        break;
    }
}

int main(void)
{

    int msqid;
    key_t key;

    if ((key = ftok("server_main.c", 'A')) == -1)
    { // Same path and same project identifier will generate the same key
        perror("Error in ftok\n");
        exit(1);
    }

    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1)
    { // read-write permission
        perror("Error in msgget\n");
        exit(2);
    }

    while (1)
    {
        struct my_msgbuf buf;
        buf.mtype = 1;
        if (msgrcv(msqid, &buf, sizeof(buf) - sizeof(long), 1, 0) == -1)
        {
            perror("Error in msgrcv\n");
            exit(1);
        }

        if (buf.service_option == 5)
        {
            while (wait(NULL) > 0)
            {
                // to wait till child processes terminated
            };

            if (msgctl(msqid, IPC_RMID, NULL) == -1)
            {
                perror("msgctl");
                exit(1);
            }
            printf("Main server terminated gracefully.\n");
            exit(0);
            goto skip;
        }
        else
        {
            printf("Handling request by client id number %d\n",buf.cid);
            pid_t pid; // Variable to store the process ID

            // Fork a new process
            pid = fork();

            // Check if fork was successful
            if (pid < 0)
            {
                fprintf(stderr, "Fork failed!\n");
                return 1;
            }

            if (pid == 0)
            {

                handleClientRequest(buf, msqid);
                // This code block is executed by the child process
            }
            else
            {
                // This code block is executed by the parent process
                wait(NULL);
            }
        }
    }
    skip:
    return 0;
}
