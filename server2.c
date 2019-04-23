#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<ctype.h>
#include<sys/stat.h>

#include<sys/sendfile.h>

#include<fcntl.h>


void request(int newsockfd);
void error(const char *msg){
    perror(msg);
    exit(1);
}
int sockfd,newsockfd,port_no, i, pid, n, size, status = 0,loggedin=0;
    
    struct stat obj,st = {0};
    struct sockaddr_in serv_addr,client_addr;
    socklen_t clilen;
    char buffer[100]= {0}, command[4]= {0}, filename[20]= {0}; //changed
    char username[20] = {0};
	char password[20]= {0};
	char firstname[20]= {0};
	char pass_verify[20]= {0};
    char string_0[256]= {0};
    int filehandle;

int main(int argc,char *argv[]){
    if(argc < 2){
        fprintf(stderr,"wrong arguments");
    }

    
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        error("error in creating socket");
    }

    bzero((char *) &serv_addr,sizeof(serv_addr));

    port_no = atoi(argv[1]);

    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);

    if(bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("error in bind");
    

    listen(sockfd,5);
    clilen = sizeof(client_addr);
    while(1){
        newsockfd = accept(sockfd,(struct sockaddr *) &client_addr, &clilen);
        if(newsockfd < 0){
        error("error in creating newsockfd");
        }
        pid = fork();
        if(pid < 0 )
            error("error in fork");
        if(pid == 0){
            //close(sockfd);
        while(1){
                request(newsockfd);
        }
        }else{
            close(newsockfd);
        }
    }
    close(sockfd);
}



    void request(int newsockfd){
        
       n = read(newsockfd,buffer,100);
        if(n < 0)
            error("error in reading\n");
        sscanf(buffer,"%s",command);
        printf("command recieved : %s \n",command);
        if(strcmp(command,"CRET") == 0){
            FILE *filePointer;
            filePointer = fopen("login_info.txt","a");
            fputs(buffer+5,filePointer);
            fputs("\n",filePointer);
            fclose(filePointer);
            status = 1;
            write(newsockfd,&status,sizeof(int));
            
            }
        
        else if (strcmp(command,"USER") == 0){
            status = 0;
            FILE *filePointer;
            filePointer = fopen("login_info.txt","r+");
            sscanf(buffer+5,"%s",username);
            while((fgets(string_0,256,filePointer)) != NULL){
         
                sscanf(string_0,"%s",firstname);

                if(strcmp(firstname,username) == 0){
                    status = 331;
                    printf("A match has been found\n");
                    break;

                }else{
                    status = 332;
                }
            }
            fclose(filePointer);
            n = write(newsockfd,&status,sizeof(int));
            if(n < 0)
                error("error in writing\n");
            
            }
        else if(strcmp(command,"PASS") == 0){
            FILE *filePointer;
            filePointer = fopen("login_info.txt","r+");
            sscanf(buffer+5,"%s",password);

            while(fgets(string_0,256,filePointer) != NULL){
                sscanf(string_0,"%s %s" ,firstname,pass_verify);

                if(strcmp(firstname,username) == 0 && strcmp(pass_verify,password) == 0){
                    status = 331;
                    loggedin = 1;
                    printf("user logged in\n");
                    break;
                }
            }
            fclose(filePointer);
            n = write(newsockfd,&status,sizeof(int));
            if(n < 0){
                error("error in password writing part\n");
            }
            }
        else if(strcmp(command,"PWD") == 0){
            if(!loggedin){
                status = 530;
                write(newsockfd,&status,sizeof(int));
            }else{
                system("pwd>pwd_file.txt");
                int j = 0;
                FILE *f = fopen("pwd_file.txt","r");
                while(!feof(f)){
                    buffer[j++] = fgetc(f);
                }
                buffer[j-1] = '\0';

                fclose(f);
                write(newsockfd,buffer,100);

            }
        }
        else if(strcmp(command, "CWD") == 0){ 
			if(!loggedin){
                status = 530;
                write(newsockfd, &status, sizeof(int));
            }else{
				printf("executing command cd \n ");
                char path[50] ;
                sscanf(buffer," %*s %s",path);
                if(chdir(path) == 0){
                    status = 1;
                    printf("Directory changed\n");
                }
                else
                    status = 0;
                write(newsockfd, &status, sizeof(int));
                printf("\n%d\n",status);
                printf("\ncd executed");
                system("pwd>>temp.txt");
            }
        }
        else if(strcmp(command, "RMD") == 0){
            if(!loggedin){
                status = 530;
                write(newsockfd, &status, sizeof(int));
            }else{
                printf("executing command rmdir \n ");
                char path[50] ;
                sscanf(buffer," %*s %s",path);
                if(rmdir(path) == 0){
                    status = 1;
                    printf("Directory Deleted\n");
                }
                else{
                    status = 0;
                }
                write(newsockfd, &status, sizeof(int));
                printf("\ncd executed");
            }
        }
        else if(strcmp(command, "MKD") == 0){
            if(!loggedin){
                status = 530;
                write(newsockfd, &status, sizeof(int));
            }else{
                printf("Executing command mkdir \n");
                char path[50] ;
                sscanf(buffer," %*s %s",path);
                if(mkdir(path,0777) == 0){
                    status = 1;
                    printf("Directory created\n");
                }
                else{
                    status = 0;
                }
                write(newsockfd, &status, sizeof(int));
                printf("\nmkdir executed");
            }
        }
        else  if(!strcmp(command, "LIST")){
            if(!loggedin){ 
                status = 530;
                write(newsockfd, &status, sizeof(int));
                }else{
                    system("ls >list_file.txt");
                    i = 0;
                    stat("list_file.txt",&obj);
                    size = obj.st_size;
                    write(newsockfd, &size, sizeof(int));
                    filehandle = open("list_file.txt", O_RDONLY);
                    sendfile(newsockfd,filehandle,NULL,size);
                }
        }
        else if(strcmp(command, "STOR") == 0){
            if(!loggedin){
                status = 530;
                write(newsockfd, &status, sizeof(int));
            }else{
                // status = 200;
                printf("Receiving the file from client : \n");
                FILE *fp;
                int ch = 0;
                fp = fopen("received.txt","a");           
                int words;
                
                read(newsockfd, &words, sizeof(int));
                printf("%d\n",words);
                while(ch < words){
                    // printf("%s ",buffer);
                    read(newsockfd ,buffer,sizeof(buffer)); 
                    fprintf(fp , "%s " , buffer);
                    ch++;
                }
                fputs("\n",fp);
                
                fclose(fp);
                printf("The file was received successfully\n");
                
            }
        }

        // else if(strcmp(command,"RETR") == 0){
        //     if(!loggedin){
        //         status = 530;
        //         write(newsockfd,&status,sizeof(int));
        //     }else{
        //         sscanf(buffer,"%*s %s",filename);
        //         printf("filename: %s",filename);
        //         stat(filename,&obj);
        //         filehandle = open(filename, O_RDONLY);
        //         size = obj.st_size;
        //         if(filehandle == -1)
        //             size = 0;
        //         write(newsockfd,&size,sizeof(int));
        //         if(size)
        //             sendfile(newsockfd,filehandle,NULL,size);

        //     }
        // }
        else if(strcmp(command, "RETR") == 0){
            if(!loggedin){
                status = 530;
                write(newsockfd,&status,sizeof(int));
            }else{
                status = 200;
                write(newsockfd,&status,sizeof(int));
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
     	        rewind(f);
                int ch = 0 ;
                while(ch < words){
		            fscanf(f , "%s" , buffer);
                    // printf("%s ",buffer);
                    write(newsockfd,buffer,sizeof(buffer));
                    ch++;
                }
                fclose(f);
	            printf("The file was sent successfully. \n");
            }
        }
        else if(strcmp(command, "ABOR") == 0){
            loggedin = 0;
            i = 0;
            write(newsockfd, &i, sizeof(int));
        }
        else if(strcmp(command, "QUIT") == 0){
            printf("FTP server quitting..\n");
            i = 1;
            write(newsockfd, &i, sizeof(int));
            exit(0);
        }else{
            char *msg = "no such command ";
            write(newsockfd,&msg,sizeof(msg));
        }
    }




