#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "ParseConf.c"
 
#define SERVER_TCP_PORT 3000
#define    BUFLEN 500
#define MAX_LINE_LENGTH 512
 
int main(int argc, char *argv[])
{
    char    *host = "localhost"; /* host to use if none supplied */
    char    *bp, sbuf[BUFLEN], rbuf[BUFLEN], path[BUFLEN];
    int     port, bytes_to_read;
    int        i, sd, n, quit = 1; /* socket descriptor and socket type */
    FILE    *fp;
    FILE    *user_file;
    char temp;
    char temp2;
    char response[256];
    char line[MAX_LINE_LENGTH] = {0};
 
 
    char * file;
    char *value;
    char *value2;
 
    struct stat fstat;
    struct hostent *hp; /* pointer to host information entry */
    struct sockaddr_in server; /* an Internet endpoint address */
    struct PDU {
        char type;
        int length;
        char data[BUFLEN];
    } rpdu, tpdu;
 
    switch(argc){
        case 3:
            host = argv[1];
            port = atoi(argv[2]);
        break;
        case 4:
            host = argv[1];
            port = atoi(argv[2]);
            file = argv[3];
            /* Create a stream socket */
            if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "Can't create a socket\n");
                exit(1);
            }
 
            bzero((char *)&server, sizeof(struct sockaddr_in));
            server.sin_family = AF_INET;
            server.sin_port = htons(port);
            if (hp = gethostbyname(host))
                bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
            else if (inet_aton(host, (struct in_addr *) &server.sin_addr)){
                fprintf(stderr, "Can't get server's address\n");
                exit(1);
            }
 
            /* Connecting to the server */
            if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
                fprintf(stderr, "Can't connect \n");
                exit(1);
            }
 
            user_file = fopen (file, "r");
            if(user_file){
 
                printf("File contents\n");
 
    int flag = 1;
 
 
    while(fgets(line, MAX_LINE_LENGTH, user_file)){
        char tempc [MAX_LINE_LENGTH] = {0};
        int i=0,j=0;
        while( i <= MAX_LINE_LENGTH ){
            tempc[i] = ' ';
            i++;
        }
 
        while(line[j] != ';'){
            if(j< 2){
                tempc[j] = line[j];
        }
        else{
            tempc[j+6] = line[j];
        }
        j++;
        }
 
 
 
 
        tempc[j+6] = '\0';
 
               write(sd, (char *)&tempc, sizeof(tempc)); // request file list*/
    //write(sd, (char *)&line, sizeof(line)); // request file list
        
        read(sd, (char *)&rpdu, sizeof(rpdu)); // receive file list
        if(tempc[0] == 'L'){
                write(1, rpdu.data, rpdu.length); // display to stdout
        }
        else if(tempc[0] == 'Q'){
            exit(0);
        }
 
        else{
            i = 0;
        while(i<j){
            tempc[i] = tempc[i+8];
            i++;
        }
        tempc[i] = '\0';
        printf(" Downloading...!\n");
 
 
        i = 0;
        while (i< BUFLEN || tempc[i] != '\0'){
            tpdu.data[i] = tempc[i];
            i++;
        }
        tpdu.length = sizeof(tempc)/sizeof(tempc[0]);
 
        printf("=== |%s|\n",tempc);
                
        tpdu.data[tpdu.length-1] = '\0';
        write(sd, (char *)&tpdu, sizeof(tpdu));
        bytes_to_read = read(sd, (char *)&rpdu, sizeof(rpdu));
        fp = fopen(tpdu.data, "w");
        fwrite(rpdu.data, sizeof(char), rpdu.length, fp); // write data to file
            while (bytes_to_read != 0 && rpdu.length == BUFLEN) { // if there is more data to write
            read(sd, (char *)&rpdu, sizeof(rpdu));
            fwrite(rpdu.data, sizeof(char), rpdu.length, fp);
            bytes_to_read -=rpdu.length;
        }
        fclose(fp);
            printf("Transfer sucessful.\n");
                    }
                }
                     }
 
            else{
                printf("File does not exist\n");
                exit(-1);
            }
            break;
                
        default:
            fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
            exit(1);
    }
 
    /* Create a stream socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't creat a socket\n");
        exit(1);
    }
 
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (hp = gethostbyname(host))
        bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
    else if (inet_aton(host, (struct in_addr *) &server.sin_addr)){
        fprintf(stderr, "Can't get server's address\n");
        exit(1);
    }
 
    /* Connecting to the server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
        fprintf(stderr, "Can't connect \n");
        exit(1);
    }
    
    while (quit) {
        printf("Download (D), List Directory (L), Quit (Q):\n");
        scanf(" %c", &tpdu.type);
        tpdu.length = 0; // set default data unit length
 
        switch(tpdu.type) {
            /* File Download */
            case 'D':
                printf("Enter filename: \n");
                tpdu.length = read(0, tpdu.data, BUFLEN-1); // get user message
                tpdu.data[tpdu.length-1] = '\0';
                write(sd, (char *)&tpdu, sizeof(tpdu));
                read(sd, (char *)&rpdu, sizeof(rpdu));
    
                if (rpdu.type == 'F') {
                    fp = fopen(tpdu.data, "w");
                    fwrite(rpdu.data, sizeof(char), rpdu.length, fp); // write data to file
                    while (rpdu.length == BUFLEN) { // if there is more data to write
                        read(sd, (char *)&rpdu, sizeof(rpdu));
                        fwrite(rpdu.data, sizeof(char), rpdu.length, fp);
                    }
                    fclose(fp);
                    printf("Transfer sucessful.\n");
                } else {
                    fprintf(stderr, "%s", rpdu.data);
                }
                break;
 
        
 
            /* List Files */
            case 'L':
                write(sd, (char *)&tpdu, sizeof(tpdu)); // request file list
                read(sd, (char *)&rpdu, sizeof(rpdu)); // recieve file list
                write(1, rpdu.data, rpdu.length); // display to stdout
                break;
    
            /* Quit */
            case 'Q':
                write(sd, (char *)&tpdu, sizeof(tpdu)); // request file list
                quit = 0;
                break;
        }
    }
    close(sd);
    return(0);
}
