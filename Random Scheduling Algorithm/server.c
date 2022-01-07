#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "ParseConf.c"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
 
 
 
#define SERVER_TCP_PORT 3000
#define BUFLEN 500
#define sched 4
pthread_mutex_t lock;
double time_spent = 0.0;
time_t s, val = 1;
struct tm* current_time;
clock_t begin;
 
 
void *connection_handler(void *);
void IP_formatter(char *IPbuffer);
void check_host_name(int hostname);
void check_host_entry(struct hostent * hostentry);
 
int main(int argc, char **argv)
{
    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
    
    char *filePath=argv[1];
    Lines=ParseConf(filePath);
    displayKeyValue(Lines);
 
    int     sd, new_sd, client_len, port, numberofConnections, n, numberofThreads;
    struct    sockaddr_in server, client;
    int     bytes_to_read, quit = 1;
    DIR        *d;
    FILE     *fp;
 
    struct sockaddr_in their_addr; // connector.s address information
 
    struct dirent *dir;
    struct stat fstat;
    struct PDU {
        char type;
        int length;
        char data[BUFLEN];
    } rpdu, spdu;
    int server_socket;
 
    
    port = atoi(getValue(Lines,"port"));
    numberofConnections = atoi(getValue(Lines,"Connections"));
    numberofThreads = atoi(getValue(Lines,"Threads"));
    if(opendir(getValue(Lines, "Directory"))){
        chdir(getValue(Lines, "Directory"));
        printf("Directory changed to \"%s\"\n", (getValue(Lines, "Directory")));
        spdu.type = 'R';
    }
    else{
        printf("Invalid directory");
        
    }
    char *Schedule = getValue(Lines, "Schedule");
    
   /* switch(argc){
        case 1:
            port = SERVER_TCP_PORT; //default port
            break;
        case 6:
            port = atoi(argv[1]);
            numberofThreads = atoi(argv[2]);
            numberofConnections = atoi(argv[3]);
            if(opendir(argv[5])){
                chdir(argv[5]);
                printf("Directory changed to \"%s\"\n", (argv[5]));
                spdu.type = 'R';
            }
            else{
                printf("Invalid directory");
            }
            break;
        default:
            fprintf(stderr, "Usage: portNumber Threads Connections Schedule MediaFolder" );
            exit(-1);
    } */
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
 
    if (server_socket==-1){
    perror("Socket initialisation failed");
    exit(EXIT_FAILURE);
    }
    else
    printf("Server socket created successfully\n");
 
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    //bind the socket to the specified IP addr and port
    if (bind(server_socket, (struct sockaddr*) &server_addr, sizeof(server_addr))!=0){
    printf("socket bind failed...\n");
        exit(0);
    }
    else
    printf("Socket successfully binded..\n");
    
    if (listen(server_socket, numberofConnections)!=0){
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    
    int no_threads=0, policy;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t threads[numberofThreads];
    if(pthread_attr_getschedpolicy(&attr, &policy) != 0){
        fprintf(stderr, "Unable to get policy\n");
    }
 
    if (pthread_attr_setschedpolicy(&attr, SCHED_OTHER) != 0)//change  
											Schedule 
    fprintf(stderr, "Unable to set policy.\n");
    if(pthread_mutex_init(&lock, NULL) != 0){
        printf("Mutex init failed\n");
        return 1;
    }
    while (no_threads<numberofThreads){
    printf("Listening...\n");
       int sin_size = sizeof(struct sockaddr_in);
    int client_socket = accept(server_socket,(struct sockaddr *)&their_addr,&sin_size);
        char host[256];
           char *IP;
           struct hostent *host_entry;
           int hostname;
           hostname = gethostname(host, sizeof(host)); //find the host name
           check_host_name(hostname);
           host_entry = gethostbyname(host); //find host information
           check_host_entry(host_entry);
           IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
           //Convert into IP string
           printf("Current Host Name: %s\n", host);
           printf("Host IP: %s\n", IP);
        printf("Received request from Client: %s:%d\n",
               inet_ntoa(their_addr.sin_addr),port);
    
    puts("Connection accepted");
        printf("Client %d\n", numberofConnections);
        s = time(NULL);
        printf("Arrival Time: ");
        current_time = localtime(&s);
        printf("%02d:%02d:%02d\n",
               current_time->tm_hour,
               current_time->tm_min,
               current_time->tm_sec);
       begin = clock();
 
        
    if( pthread_create( &threads[no_threads], &attr,  connection_handler , &client_socket) < 0){
    perror("Could not create thread");
    return 1;}
        if (client_socket < 0) {
            printf("server accept failed...\n");
            exit(0);
            }
        else
            printf("Server accept the client...\n");
    puts("Handler assigned");
    no_threads++;
        numberofConnections--;
 
    }
    int k=0;
    for (k=0;k<numberofThreads;k++){
        pthread_join(threads[k],NULL);
        pthread_mutex_destroy(&lock);
    }
 
    //int send_status;
    //send_status=send(client_socket, server_message, sizeof(server_message), 0);
    close(server_socket);
    freeConf(Lines);
 
    return 0;
}
 
 
void *connection_handler(void *client_socket){
    int sd = *(int*) client_socket;
    int     bytes_to_read, quit = 1, n;
    DIR        *d;
    FILE     *fp;
 
    struct dirent *dir;
    struct stat fstat;
    struct PDU {
        char type;
        int length;
        char data[BUFLEN];
    } rpdu, spdu;
 
    while (quit) {
        read(sd, (char *)&rpdu, sizeof(rpdu)); // data from client
 
        switch(rpdu.type) {
            /* Download request */
            case 'D':
                read(sd, (char*)&rpdu, sizeof(rpdu));
                fp = fopen(rpdu.data, "r");
                if (fp == NULL) {
                    spdu.type = 'E'; // data unit type as error
                    printf("Error 999 File \"%s\" not found\n", rpdu.data);
                    spdu.length = strlen(spdu.data);
                    write(sd, (char *)&spdu, sizeof(spdu));
                    fprintf(stderr, "Error: File \"%s\" not found.\n", rpdu.data);
                } else {
                    spdu.type = 'F';
                    stat(rpdu.data, &fstat); // get file size
                    write(sd, (char *)&rpdu, sizeof(rpdu));
                    bytes_to_read = fstat.st_size;
                    while(bytes_to_read > 0) {
                        spdu.length = fread(spdu.data, sizeof(char), BUFLEN, fp);
                        bytes_to_read -= spdu.length;
                        spdu.type = 'F';
                        write(sd, (char *)&spdu, sizeof(spdu));
                    }
                    printf("100\n\n");
                    printf("Transfer of \"%s\" sucessful.\n", rpdu.data);
                    fseek(fp, 0L, SEEK_END);
                    long int res = ftell(fp);
                    printf("Size of the file is %ld bytes \n", res);
 
                }
 
                break;
 
 
            /* List files */
            case 'L':
                d = opendir("./"); // open current directory
                spdu.data[0] = '\0';
                while ((dir = readdir(d)) != NULL) { // read contents
                    strcat(spdu.data, dir->d_name);
                    strcat(spdu.data, "\n");
                }
                closedir(d); // close directory
                spdu.length = strlen(spdu.data);
                write(sd, (char *)&spdu, sizeof(spdu)); // send list to client
                printf("List files.\n");
 
                break;
 
            /* Quit */
            case 'Q':
                printf("Client has terminated connection\n");
 
                //exit(0);
                clock_t end = clock();
                time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
                printf("Time Elapsed: ");
                printf("%f\n", time_spent);
                printf("Exit Time: ");
                printf("%02d:%02d:%02d\n",
                       current_time->tm_hour,
                       current_time->tm_min,
                       current_time->tm_sec);
                quit = 0;
                pthread_exit(0);
                break;
        }
    }
    close(sd); // close TCP connection
}
void check_host_name(int hostname) { //This function returns host name for local computer
   if (hostname == -1) {
      perror("gethostname");
      exit(1);
   }
}
void check_host_entry(struct hostent * hostentry) { //find host info from host name
   if (hostentry == NULL) {
      perror("gethostbyname");
      exit(1);
   }
}
void IP_formatter(char *IPbuffer) { //convert IP string to dotted decimal format
   if (NULL == IPbuffer) {
      perror("inet_ntoa");
      exit(1);
   }
}
