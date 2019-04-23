#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<netdb.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>
#include<ctype.h>


void error(const char *msg){
    perror(msg);
    exit(1);
}
void errormsg(int error){
    if(error==530)
        printf("530 :YOU are not logged in\n ");
    else if(error==331)
        printf("331 :username exists password needed\n ");
    else if(error==332)
        printf("332 :NO USER FOUND\n ");
}

int main(int argc,char *argv[]){
    if(argc < 3){
        fprintf(stderr,"wrong arguments");
    }

    int sockfd, port_no, i, n, size, status = 0, filehandle;
    struct sockaddr_in serv_addr;
    char buffer[100],command[5],filename[20],input[50],*f;
    struct hostent *server;
    

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        error("error in creating host");
    }
    port_no = atoi(argv[2]);

    server = gethostbyname(argv[1]);
    if(server == NULL){
        fprintf(stderr,"no such host");
    }

    bzero((char *) &serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr_list[0],(char *) &serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port_no);

    //serv_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)server->h_addr_list[0])));
    


    if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("connection failed");
    i = 1;
    printf("1. CRET : create user\n2.USER : Enter the username for login\n3.PASS : Enter the password\n4.PWD : Print the current directory of server\n5. CWD : change the directory of host\n6. MKD : make directory on host\n7. RMD : Remove directory from server\n8.LIST: List the file of host directory\n9.STOR : send the file to host\n10. RETR : retrieve file from host\n\n\n");

    while(i){

        bzero(input,50);
        bzero(buffer,100);
        fgets(input,50,stdin);
        if(strncmp(input,"CRET",4) == 0){
                strcpy(buffer,input);
                write(sockfd,buffer,sizeof(buffer));
                read(sockfd,&status,sizeof(int));
                //printf("status: %d",status);
                if(status == 1)
                    printf("user created successfully\n");
                else
                    printf("error in user creation\n");
            }
        
        else if(strncmp(input,"USER",4) == 0){
            strcpy(buffer,input);
            n = write(sockfd,buffer,100);
            if(n < 0)
                error("error in sending user");
            n = read(sockfd,&status,sizeof(int));
            if(n < 0)
                error("error in recieving user");
            if(status)
                errormsg(status);
            else
                printf("error\n");
            
            }
        else if(strncmp(input,"PASS",4) == 0){
            strcpy(buffer,input);
            n = write(sockfd,buffer,100);
            if(n < 0)
                error("error in sending pass");
            n = read(sockfd,&status,sizeof(int));
            if(n < 0)
                error("error in recieving pass");
            //if(status)
            //    errormsg(status);
            //else
            //    printf("error\n");
            }
        else if(strncmp(input,"PWD",3) == 0){
                strcpy(buffer,input);
                write(sockfd,buffer,100);
                read(sockfd,buffer,100);
                if(buffer == 530)
                    errormsg(530);
                else{
                    printf("the path of the remote directory is: %s\n",buffer);
                }
            }
        else if(strncmp(input,"CWD",3)==0){
                strcpy(buffer, input);
                write(sockfd, buffer, 100);
                printf("\ndata sent");
                read(sockfd, &status, sizeof(int));
                if(status==1)
                    printf("Remote directory successfully changed\n");
                else
                    errormsg(status);
            }
        else if(strncmp(input,"RMD",3)==0){
            strcpy(buffer,input);
            write(sockfd,buffer,100);
            read(sockfd,&status,sizeof(int));
            if(status==1)
                    printf("Remote directory successfully Deleted\n");
                else
                    errormsg(status);
        }
        else if(strncmp(input,"MKD",3)==0){
            strcpy(buffer,input);
            write(sockfd,buffer,100);
            read(sockfd,&status,sizeof(int));
            if(status==1)
                    printf("Remote directory successfully Created\n");
                else
                    errormsg(status);
        }
        else if(strncmp(input,"LIST",4)==0){
                strcpy(buffer, "LIST");
                write(sockfd, buffer, 100);
                read(sockfd, &size, sizeof(int));
                if(size==530){
                    errormsg(size);
                }else{
                    f = malloc(size);
                    read(sockfd, f, size);
                    filehandle = creat("list_file.txt", O_WRONLY);
	                write(filehandle, f, size);
                    close(filehandle);
                    printf("The remote directory listing is as follows:\n");
                    system("cat list_file.txt");
			}
	}
        else if(strncmp(input,"STOR",4) == 0){
            strcpy(buffer,input);
            write(sockfd,buffer,100);
            
            FILE *f;
            int words = 0;
            char c;
            char file[50];
            sscanf(buffer, " %*s %s",file);
            f=fopen(file,"r");
            while((c=getc(f))!=EOF){
	            fscanf(f , " %s" , buffer);
                if(isspace(c)||c=='\t'){
                    words++;
                }	
	        }
	        write(sockfd, &words, sizeof(int));
            printf("%d\n",words);
     	    rewind(f);
            int ch = 0 ;
            while(ch<words){
		        fscanf(f , "%s" , buffer);
                // printf("%s ",buffer);
                write(sockfd,buffer,sizeof(buffer));
                // ch = fgetc(f);
                ch++;
            }
            fclose(f);
	        printf("The file was sent successfully. \n");
            //i=0;
        }
        // else if(strncmp(input,"RETR",4) == 0){
        //         strcpy(buffer,input);
        //         write(sockfd,buffer,100);
        //         sscanf(input,"%*s %s",filename);
        //         printf("the filename is %s",filename);
        //         read(sockfd,&size,sizeof(int));
        //         if (size == 530){
        //             errormsg(size);
        //         }
        //         else if (!size){
        //             error("no such file in remote directory");
            
        //         }
        //         f = malloc(size);
        //         read(sockfd,f,size);
        //         while(1){
        //             filehandle  = open(filename ,O_CREAT | O_EXCL | O_WRONLY, 0666);
        //             if(filehandle == -1){
        //                 sprintf(filename + strlen(filename),"%d",i);
        //             }
        //             else break;
                
        //         }
        //         write(filehandle,f,size);
        //         close(filehandle);
        //         strcpy(buffer,"cat ");
        //         strcat(buffer,filename);
        //         system(buffer);
        //     }
        else if(strncmp(input,"RETR",4)==0){
            strcpy(buffer,input);
            write(sockfd,buffer,100);
            read(sockfd,&status,sizeof(int));
            printf("Receiving the file from server : \n");
            FILE *fp;
            int ch = 0;
            fp = fopen("RetFROMServer.txt","a");
            int words;
            read(sockfd, &words, sizeof(int));
            while(ch < words){
                // printf("%s ",buffer);
                read(sockfd ,buffer,sizeof(buffer)); 
                fprintf(fp , "%s " , buffer);
                ch++;
            }
            fclose(fp);
            printf("The file was received successfully\n");
        }
        else if(strncmp(input,"ABOR",4)==0){
                    strcpy(buffer, input);
                    write(sockfd, buffer, 100);
                    read(sockfd, &status, 100);
                    if(status){
                        printf("Session reset\n");
                    }else{
                        errormsg(status); 
                    }
            }

        else if(strncmp(input,"QUIT",4)==0){
                strcpy(buffer, input);
                write(sockfd, buffer, 100);
                read(sockfd, &status, 100);
                if(status){
                    printf("Server closed\nQuitting..\n");
                    exit(0);
                }
                printf("Server failed to close connection\n");
            }else{
                status  = 502;
                errormsg(status);
            } 
        


    }

    close(sockfd);
}

