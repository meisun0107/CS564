// Copyright 2021 Mei Sun
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool onlySpaces(char *buffer);
void cmd_execute(char *cmd[]);
void cmd_redirection_execute(char *cmd[], char *fileName[]);
void shell_execute(FILE *fp, char *buffer);
bool mode = false;

void printAlias();
void insertNode(char *alias, char *cmd);
void alias_execute(char *buffer);
void unalias_execute(char *buffer);
char *findNode(char *alias);

struct node {
    char name[128];
    char commendLine[512];
    struct node *next;
};
struct node *head = NULL;
struct node *current = NULL;
struct node *prev = NULL;
void removeNode(char *alias);

// display all alias
void printAlias() {
    struct node *ptr = head;
    // start from the beginning
    while (ptr != NULL) {
        fprintf(stdout, "%s %s\n", ptr->name, ptr->commendLine);
        fflush(stdout);
        ptr = ptr->next;
    }
}

// insert into the alias
void insertNode(char *alias, char *cmd) {
    struct node *link = (struct node*) malloc(sizeof(struct node));
    strcpy(link->name, alias);
    strcpy(link->commendLine, cmd);
    link->next = NULL;
    // If head is empty, create new head
    if (head == NULL) {
        head = link;
        return;
    }
    current = head;
    // move to the end of the list
    while (current->next != NULL) {
        // if the alias already exists, replace it with the new one
        if (strcmp(current->name, alias) == 0) {
            strcpy(current->commendLine, cmd);
            return;
        }
        current = current->next;
    }
    if (strcmp(current->name, alias) == 0) {
            strcpy(current->commendLine, cmd);
            return;
    }
    current->next = link;
}

// remove from the alias
void removeNode(char *alias) {
    if (head == NULL) {
        return;
    }
    if (strcmp(head->name, alias) == 0) {
        if (head->next != NULL) {
            head = head->next;
            return;
        } else {
            head = NULL;
            return;
        }
    }
    // do not exists such a node
    if (strcmp(head->name, alias) != 0 && head->next == NULL) {
        return;
    }
    current = head;
    while (current->next != NULL && strcmp(current->name, alias) != 0) {
        prev = current;
        current = current->next;
    }
    if (strcmp(current->name, alias) == 0) {
        prev->next = prev->next->next;
        free(current);
    } else {
        return;
    }
}

char *findNode(char *alias) {
    if (head == NULL) {
        // did not find the alias
        return NULL;
    }
    current = head;
    while (current->next != NULL) {
        if (strcmp(current->name, alias) == 0) {
            return(current->commendLine);
        }
        current = current->next;
    }
    if (strcmp(current->name, alias) == 0) {
        return(current->commendLine);
    }
    return NULL;
}

void alias_execute(char *buffer) {
    // alias
    char *tok = strtok(buffer, " \t");
    // second arg - alias-name
    tok = strtok(NULL, " \t");
    if (tok == NULL) {
        printAlias();
        return;
    }
    if (strcmp(tok, "alias") == 0) {
        fprintf(stderr, "alias: Too dangerous to alias that.\n");
        fflush(stdout);
        return;
    }
    if (strcmp(tok, "unalias") == 0) {
        fprintf(stderr, "alias: Too dangerous to alias that.\n");
        fflush(stdout);
        return;
    }
    if (strcmp(tok, "exit") == 0) {
        fprintf(stderr, "alias: Too dangerous to alias that.\n");
        fflush(stdout);
        return;
    }
    char name[128] = {0};
    strcat(name, tok);
    // third arg
    tok = strtok(NULL, " \t");
    // only 2 arguments
    if (tok == NULL) {
        if (findNode(name) != NULL) {
            fprintf(stdout, "%s %s\n", name, findNode(name));
            fflush(stdout);
            return;
        }
        return;
    }
    char newBuffer[512] = {0};
    while (tok != NULL) {
         strcat(newBuffer, tok);
         tok = strtok(NULL, " \t");
         if (tok != NULL) {
             strcat(newBuffer, " ");
         }
    }
    insertNode(name, newBuffer);
}

void unalias_execute(char *buffer) {
    char *tok = strtok(buffer, " \t");
    tok = strtok(NULL, " \t");
    // deal with dangers input
    if (tok == NULL) {
        fprintf(stderr, "unalias: Incorrect number of arguments.\n");
        fflush(stdout);
        return;
    }
    // store the name of alias to remove
    char name[128] = {0};
    strcpy(name, tok);
    // get the next argument from buffer, if there are more arguments, return
    tok = strtok(NULL, " \t");
    if (tok != NULL) {
      fprintf(stderr, "unalias: Incorrect number of arguments.\n");
      fflush(stdout);
      return;
    }
    if (findNode(name) == NULL) {
        return;
    } else {
        removeNode(name);
        return;
    }
}
void shell_execute(FILE *fp, char *buffer) {
    while (fgets(buffer, 512, fp) != NULL) {
        if (mode == true) {
            write(STDOUT_FILENO, buffer, strlen(buffer));
        }
        if (strcmp(buffer, "\n") == 0 || onlySpaces(buffer) == true) {
            continue;
        }
        if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n')) {
            buffer[strlen(buffer) - 1] = '\0';
        }
        if (strcmp(buffer, "exit") == 0) {
            exit(0);
        }
        if (strncmp(buffer, "alias", 5) == 0) {
            alias_execute(buffer);
            if (mode == false) {
              write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
            }
            continue;
        }
        if (strncmp(buffer, "unalias", 7) == 0) {
            unalias_execute(buffer);
            if (mode == false) {
              write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
            }
            continue;
        }
        if (strstr(buffer, ">") == NULL) {
            // alias
            if (findNode(buffer) != NULL) {
                strcpy(buffer, findNode(buffer));
                if (buffer[strlen(buffer) - 1] == '\n') {
                    buffer[strlen(buffer) - 1] = '\0';
                }
            }
            int i = 0;
            char *tok = strtok(buffer, " \t");
            char *cmd[10];
            while (tok != NULL) {
               cmd[i] = tok;
               tok = strtok(NULL, " \t");
               i++;
            }
            cmd[i] = NULL;
            cmd_execute(cmd);
        } else {  // redirction
            int num = 0;
            char *token = strtok(buffer, ">");
            char *seperate[2];
            while (token != NULL) {
              if (num < 2) {
                 seperate[num] = token;
               }
               token = strtok(NULL, ">");
               num++;
            }
            char misFormat[] = "Redirection misformatted.\n";
            if (num != 2) {
                write(STDERR_FILENO, misFormat, strlen(misFormat));
                if (mode == false) {
                  write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
                }
                continue;
            }
            if (seperate[1] == NULL) {
                write(STDERR_FILENO, misFormat, strlen(misFormat));
                if (mode == false) {
                    write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
                }
                continue;
            }
            int numfiles = 0;
            char *tok1 = strtok(seperate[1], " \t");
            char *fileName[10];
            while (tok1 != NULL) {
               fileName[numfiles] = tok1;
               tok1 = strtok(NULL, " \t");
               numfiles++;
            }
            if (numfiles > 1) {  // multiple files
                write(STDERR_FILENO, misFormat, strlen(misFormat));
                if (mode == false) {
                    write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
                }
                continue;
            }
            int j = 0;
            char *tok0 = strtok(seperate[0], " \t");
            char *cmd1[10];
            while (tok0 != NULL) {
               cmd1[j] = tok0;
               tok0 = strtok(NULL, " \t");
               j++;
            }
            cmd1[j] = NULL;
            cmd_redirection_execute(cmd1, fileName);
        }
        // print shell prompt
        if (mode == false) {
            write(STDOUT_FILENO, "mysh> ", strlen("mysh> "));
        }
    }
}

// execution function for the commends
void cmd_execute(char *cmd[]) {
    pid_t pid = fork();
    if (pid == 0) {
        if (execv(cmd[0], cmd) != 0) {
        char str[100];
            strcpy(str, cmd[0]);
            strcat(str, ": Command not found.\n");
            write(STDERR_FILENO, str, strlen(str));
            _exit(1);
        }
    } else {
        wait(NULL);
    }
}

// execution redirction for the commends
void cmd_redirection_execute(char *cmd[], char *fileName[]) {
    pid_t pid = fork();
    if (pid == 0) {
       int fd = open(fileName[0], O_WRONLY | O_CREAT | O_TRUNC, 0644);
       if (fd < 0) {
           char err[100];
           strcpy(err, "Cannot write to file ");
           strcat(err, fileName[0]);
           strcat(err, ".\n");
           write(STDERR_FILENO, err, strlen(err));
       }
       dup2(fd, fileno(stdout));
       close(fd);
       if (execv(cmd[0], cmd) < 0) {
           char str[100];
           strcpy(str, cmd[0]);
           strcat(str, ": Command not found.\n");
           write(STDERR_FILENO, str, strlen(str));
           _exit(1);
       }
    } else {
          wait(NULL);
    }
}
// check if the line only containing spaces
bool onlySpaces(char *buffer) {
    for (int i = 0; i < strlen(buffer); i++) {
        if (isspace(buffer[i]) != 0) {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    // Interactive mode
    if (argc == 1) {
        mode = false;
        char prompt[] = "mysh> ";
        write(STDOUT_FILENO, prompt, strlen(prompt));
        char buffer[512];
        FILE *fp = stdin;
        shell_execute(fp, buffer);
        exit(0);
     } else if (argc == 2) {  // batch mode
        mode = true;
        char buffer[512];
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
           // cannot open the file
           char str[100];
           strcpy(str, "Error: Cannot open file ");
           strcat(str, argv[1]);
           strcat(str, ".\n");
           write(STDERR_FILENO, str, strlen(str));
           exit(1);
        }
       shell_execute(fp, buffer);
       exit(0);
    } else {
        char printError[] = "Usage: mysh [batch-file]\n";
        write(STDERR_FILENO, printError, strlen(printError));
        exit(1);
    }
}







