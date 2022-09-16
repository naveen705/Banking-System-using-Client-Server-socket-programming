#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

//receiving message to client packet wise
char* msg_from_client(int fd_client) {
	//receiving packet size
    int packet = 0;
    int n = read(fd_client, &packet, sizeof(int));
    //if no packet is received
    if(n <= 0) {
        shutdown(fd_client, SHUT_WR);
        return NULL;
    }
    //each packet size is 512 bytes
    char *str = (char*)malloc(packet*512);
    memset(str, 0, packet*512);
    char *previous= str;
    int i;
    //converting packets to string
    for(i = 0; i < packet; ++i) {
        int n = read(fd_client, str, 512);
        str = str+512;
    }
    return previous;
}
//sending message to client in packetwise manner
void msg_for_client(int fd_clent, char *s) {
   // deciding number of packets
    int packets = (strlen(s)-1)/512 + 1;
    int n = write(fd_clent, &packets, sizeof(int));
    int sz=packets*512;
    char message[sz];
    char *msg = (char*)malloc(packets*512);
    strcpy(msg, s);
    int i;
    for(i = 0; i < packets; ++i) {
        int n = write(fd_clent, msg, 512);
        msg += 512;
    }
}
//function for printing balance information
char *printingBalanceInfo(char *username,int fd_client,int it)
{
        //for printing available balance
	if(it==0)
	{
		//opening file to be read
		ssize_t lineread;
		FILE *fp=fopen(username,"r");
		size_t length = 0;
		char * l = NULL;
		if((lineread = getline(&l, &length, fp)) != -1)
		{
			char *previous_token;
			char *tk;
			tk=strtok(l," \n");
			previous_token=(char *)malloc(500*sizeof(char));
			
			while(tk!=NULL)
			{
				strcpy(previous_token,tk);
				tk=strtok(NULL," \n");
			}
			fclose(fp);
			return previous_token;
		}
		else
		{
			fclose(fp);
			char *balance=(char *)malloc(2*sizeof(char));
    			balance[0]='0';
    			balance[1]='\0';
    			return balance;
		}
	} 
	//for printing mini balance
	else if(it==1)
	{
		int count=0;
		//result for storing mini balance
		char *result = NULL;
		FILE *fp = fopen(username,"r");
		result = (char *)malloc(5000*sizeof(char));
	    	result[0] = '\0';
	    	//opening file to be read
	    	ssize_t lineread;
	    	char * l = NULL;
	    	size_t length = 0;
	    
		//for printing firat five transaction of the user
		while(count<5 && (lineread = getline(&l, &length, fp)) != -1)
		{
			strcat(result,l);
			count++;
		}
	
		fclose(fp);
	
		if(strlen(result)==0)
			strcpy(result,"None");
	
		return result;
	}
}
//for finding the customer 
int searching_customer(char *user)
{
	//opening login_file
	FILE *fp=fopen("login_file","r");
	char * line = NULL;
    	size_t len = 0;
    	ssize_t read;
	//looking fo the user in the login file
	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," ");
		if(strcmp(token,user)==0)
		{
			token=strtok(NULL," ");
			token=strtok(NULL," ");
			if(token[0]=='C')
			{
				fclose(fp);
				return 1;
			}
        	}
    }

    fclose(fp);
    //if user is not found
    return 0;

}
//function for doing admin work
int operation(char *username, int client_fd)
{
	
	int flag=1;
	//providing options to the user
	msg_for_client(client_fd,"1.Press 1 to Credit\n2.Press 2 to Debit\nWrite exit to terminate the program");
	while(flag)
	{
		char* buff=msg_from_client(client_fd);
		//efrom the program
		if(strcmp(buff,"exit")==0)
			return -1;
		else
		{
			int select=atoi(buff);
			double balance=strtod(printingBalanceInfo(username,client_fd,0),NULL);
			//checking for invalid input
			if(select!=1 && select!=2)
				msg_for_client(client_fd,"Invalid Input");
			else
			{
				msg_for_client(client_fd,"Enter the amount");

				while(1)
				{
				//checking for invalid amount
					char *buff=msg_from_client(client_fd);
					double amount=strtod(buff,NULL);
					if(amount<=0)
						msg_for_client(client_fd,"Invalid amount");
					else
					{
					//checking for underflow condition
						if(select==2 && balance<amount)
						{
							msg_for_client(client_fd,"Insufficient Balance in the account.\n--------------------\n\nEnter username of the account holder or 'exit' to terminate the program.");
							flag=0;
							break;
						}
						//credit
						else if(select==1)
							balance +=amount;
						//debit
						else if(select==2)
							balance-=amount;
						char c;	
					    if(select==1)
					    	c='C';
					    else
					    	c='D';
	                    char * line = NULL;
	                    char *line_new=(char *)malloc(sizeof(char)*10000);
	                    ssize_t have_read;
                            size_t length = 0;
	                    time_t ltime; 
			     FILE *fp=fopen(username,"r");
	                    ltime=time(NULL); 
	                    sprintf(line_new,"%.*s %c %f\n",(int)strlen(asctime(localtime(&ltime)))-1,asctime(localtime(&ltime)),c,balance);

	                    while((have_read = getline(&line, &length, fp)) != -1)
	                	strcat(line_new,line);

	                    fclose(fp);
	                    fp=fopen(username,"w");
	                    fwrite(line_new, sizeof(char), strlen(line_new), fp);
	                    fclose(fp);		
			     msg_for_client(client_fd,"Account updated successfully.\n=======================\n\nEnter username of the account holder or 'exit' to terminate the program.");
			     flag=0;
			     break;
					}

				}
			}

		}
	}
}
//function for admin customer and police
void actual_function(int it,int fd_client,char *name_of_user,char * p,char *str)
{
//for customer
	if(it==0)
	{
		msg_for_client(fd_client,"Select among the options (Enter 1 or 2)\n1.Show available balance\n2.Show mini statement\nWrite exit for terminating.");
		char *buffer=NULL;
		int temp=1;
		while(temp)
		{
			if(buffer!=NULL)
				buffer=NULL;
			int choice;
			char *bal;
			bal=(char *)malloc(1000*sizeof(char));
			strcpy(bal,"====================\nAvailable Balance: ");
			buffer=msg_from_client(fd_client);
			if(strcmp(buffer,"exit")==0)
				choice=3;
			else
			    choice=atoi(buffer);
			char *str;
			str=(char *)malloc(10000*sizeof(char));
			strcpy(str,"====================\nMini Statement: \n");
			//checking available balance
			if(choice==1)
			{
				strcat(bal,printingBalanceInfo(name_of_user,fd_client,0));
				msg_for_client(fd_client,strcat(bal,"\n====================\n\nSelect your choice\n1.Available Balance\n2.Mini Statement\nWrite exit for terminating the program."));
			}
			//checking mini balance
			else if(choice==2)
			{
				strcat(str,printingBalanceInfo(name_of_user,fd_client,1));
			
				msg_for_client(fd_client,strcat(str,"\n====================\n\nEnter your choice\n1.Available Balance\n2.Mini Statement\nWrite exit for terminating the program."));
				free(str);
			}
			//terminating
			else if(choice==3)
			{
				temp=0;
			}
			else
			{
				msg_for_client(fd_client, "Invalid input");
			}
	
		}
		//closing	
		msg_for_client(fd_client, strcat(str,name_of_user));
	    shutdown(fd_client, SHUT_RDWR);	
	}
	//for admin
	else if(it==2)
	{
		msg_for_client(fd_client,"Press 1 to add new user  \nPress 2 for doing debit and credit");
		char *buffer1=NULL;
		buffer1=msg_from_client(fd_client);
		int option=atoi(buffer1);
		//for adding new users
		if(option==1)
		{
			msg_for_client(fd_client,"Enter the credentials\n==================\nEnter username");
			buffer1=msg_from_client(fd_client);
			char *username=buffer1;
			msg_for_client(fd_client,"Enter password");
			buffer1=msg_from_client(fd_client);
			char *password=buffer1;
			msg_for_client(fd_client,"Enter type (C for customer , A for admin , P for police");
			char *b;
			b=msg_from_client(fd_client);
			char *type;
			type=(char *)malloc(sizeof(char));
			type[0]=b[0];
			FILE *fp;
			if(type[0]=='C')
			{
			
			fp=fopen(username,"w");
			fclose(fp);
			}
			fp=fopen("login_file","a");
			fprintf(fp,"%s ",username);
			fprintf(fp,"%s ",password);
			fprintf(fp,"%c",type[0]);
			fprintf(fp,"\n");
			fclose(fp);
			
			
			
		}
		//for doing debit and credit
		else
		{
		msg_for_client(fd_client,"Enter username of the account holder or 'exit' to terminate the program");
		
		while(1)
		{
			char *buffer=NULL;
			buffer=msg_from_client(fd_client);
			
			
			if(strcmp(buffer,"exit")==0)
				break;
			else if(searching_customer(buffer))
			{
				char *userreq=(char *)malloc(40*sizeof(char));
				strcpy(userreq,buffer);
			
				if(operation(userreq,fd_client)==-1)
					break;
			}
			else
			msg_for_client(fd_client,"Username provided is invalid . Please enter a valid user");
		}
		msg_for_client(fd_client, strcat(str,name_of_user));
               shutdown(fd_client, SHUT_RDWR);
       }
   }
   //for police
       else if(it==1)
       {
       //police can check mini statement and available balance of all the customers
       	msg_for_client(fd_client, "Enter your choice\n1.Print Balance of all users\n2.Get mini Statement\nWrite exit to terminate");
	               int flag=1;

	            while(flag)
	            {
		          char *buffer=NULL;
		          char * str;
		          buffer=msg_from_client(fd_client);
		          char *bal;
		          bal=(char *)malloc(1000*sizeof(char));
		          str=(char *)malloc(10000*sizeof(char));
		          strcpy(bal,"------------------\nAvailable Balance: \n");
		          strcpy(str,"------------------\nMini Statement: \n");
		          if(strcmp(buffer,"exit")==0)
			         break;
		          else
		          {
			        int choice=atoi(buffer);
			        if(choice==1)
			        {
			          
			          
			          FILE *fp=fopen("login_file","r");
					  char * line = NULL;
    				  size_t len = 0;
   					  ssize_t read;
   					  char *retstr=(char *)malloc(10000*sizeof(char));
    				  retstr[0]='\0';

				 	  while((read = getline(&line, &len, fp)) != -1) 
					  {
							char *token=strtok(line," ");
							char *token1=strtok(NULL," ");
							char *token2=strtok(NULL," ");
							if(token2[0]=='C')
							{
								strcat(retstr,token);
								strcat(retstr," ");
								strcat(retstr,printingBalanceInfo(token,fd_client,0));
								strcat(retstr,"\n");
       						 }
   					 }
				      strcat(bal,retstr);
				      msg_for_client(fd_client,strcat(bal,"\n--------------------\n\nEnter your choice\n1.Print Balance of all users\n2.Get mini Statement\nWrite exit to terminate"));
			        }
			        else if(choice==2)
			        {
				       msg_for_client(fd_client,"Enter Username or exit to terminate");

				      while(1)
				      {
					    buffer=msg_from_client(fd_client);

					    if(strcmp(buffer,"exit")==0)
					    {
						  flag=1;
						  break;
					   }
					   else if(searching_customer(buffer))
					   {
						  char *username=(char *)malloc(sizeof(char)*40);
						  strcpy(username,buffer);
						  strcat(str,printingBalanceInfo(username,fd_client,1));
						  msg_for_client(fd_client,strcat(str,"\n--------------------\n\nEnter your choice\n1.Print Balance of all users\n2.Get mini Statement\nWrite exit to terminate"));
						  break;
				    	}
					   else
						  msg_for_client(fd_client,"Wrong Username. Please enter a valid user");
				      }
			       }
		           }
	           }	
			    msg_for_client(fd_client, strcat(str,name_of_user));
                shutdown(fd_client, SHUT_RDWR);
       }
}

int main(int argc,char **argv)
{
//for making connections
	int fd_socket;
	fd_socket=socket(AF_INET, SOCK_STREAM, 0);
	int fd_client;
	int p_no;
	p_no=atoi(argv[1]);
	struct sockaddr_in address_server;
	memset((void*)&address_server, 0, sizeof(address_server));
	address_server.sin_port = htons(p_no);         
	address_server.sin_family = AF_INET;             
	address_server.sin_addr.s_addr = INADDR_ANY;     
//for binding with the client
	struct sockaddr_in address_client;
	if(bind(fd_socket, (struct sockaddr*)&address_server, sizeof(address_server)) < 0) {
	    printf("Error in binding.\n");
	    exit(EXIT_FAILURE);
	}
	int utilize_again=1;
	setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &utilize_again, sizeof(int));
	listen(fd_socket, 10); 
	int sz_cli=sizeof(address_client);
//for accepting connections from client
	while(1) {
	    memset(&address_client, 0, sizeof(address_client));
	    if((fd_client = accept(fd_socket, (struct sockaddr*)&address_client, &sz_cli)) < 0) {
	        printf("Error in accept.\n");
	        exit(EXIT_FAILURE);
	    }
	//for forking the process
	    int val=fork();
	    if(val==-1)
	    	printf("Error in forking.\n");
	    else if(val==0)
	    {
	    	close(fd_socket);
	        char name_of_user[100];
	        char p[100];
	        //taking user credentials
			char *received_user;
			int user_type= -1;
			char *received_password;
			msg_for_client(fd_client,"Enter username: ");
			received_user=msg_from_client(fd_client);

			msg_for_client(fd_client,"Enter password: ");
			received_password=msg_from_client(fd_client);

			int i=0;
			while(received_user[i]!='\0' && received_user[i]!='\n')
			{
				name_of_user[i]=received_user[i];
				i++;
			}

			name_of_user[i]='\0';

			i=0;
			while(received_password[i]!='\0' && received_password[i]!='\n')
			{
				p[i]=received_password[i];
				i++;
			}
			p[i]='\0';
			size_t len = 0;
    		char * a = NULL;
    		ssize_t v;
	//checking the type of user
			FILE *fp=fopen("login_file","r");
			while((v = getline(&a, &len, fp)) != -1) 
			{
				char *words=strtok(a," ");
				if(strcmp(words,name_of_user)==0)
				{
					words=strtok(NULL," ");
					if(strcmp(words,p)==0)
					{
						words=strtok(NULL," ");
                		if(words[0]=='C')
                		{
                  			user_type=0;    
                		}
                		else if(words[0]=='P')
                		{
                		    user_type=1;
                		}
                		else if(words[0]=='A')
                		{
                    		user_type=2;
                		}	
            		}
        		}
    		}
    		if(a!=NULL)
        	free(a);
    		fclose(fp);
	        char str[60];
	        strcpy(str,"Thank You ");
	        //according to the type of users calling actual function with different values  
	        if(user_type==0)
	        {
	        	actual_function(0,fd_client,name_of_user,p,str);
				
		}
		else if(user_type==2)
		{
			actual_function(2,fd_client,name_of_user,p,str);	
		}
		else if(user_type==1)
		{
			actual_function(1,fd_client,name_of_user,p,str);
		}
		//handling unauthorised users
		else if(user_type==-1)
		{
			msg_for_client(fd_client, "Incorrect credentials");
                	shutdown(fd_client, SHUT_RDWR);
		}
		else
		{
			msg_for_client(fd_client, "Incorrect credentials");
                	shutdown(fd_client, SHUT_RDWR);
		}
		
	        exit(EXIT_SUCCESS);
		}
		else
		{
			close(fd_client);
		}

	    
	}

}
