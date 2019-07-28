#include <stdio.h>
#include <stdlib.h>


void make_command(int argc, char **argv)
{
	char command[1000];
	//cJSON *argjson=NULL;
	char buf[1000];
	memset(command,0x00,sizeof(command));
	for(int i=0;i<argc;i++)
	{
		strcat(command,argv[i]);
		strcat(command," ");
	}	
	printf("\ncommand=%s",command);				
	FILE *fp;
	char data[2000];
	 /* Open the command for reading. */
	 fp = popen(command, "r");
	 if (fp == NULL) 
	 {
		   printf("Failed to run command\n" );
		   exit(1);
	 }
	char ch;
	ch=fgetc(fp);
	int i=0;
	while(ch != EOF)
	{
		//strcat(buf,ch);
		buf[i++]=ch;
		ch=fgetc(fp);
	}	
	/*
	 while(fgets(data, sizeof(data), fp) != NULL)
	 {
	 	//argjson=cJSON_CreateObject();
		strcat(buf,data);
		//argjson=cJSON_Parse(data);
		//printf("\n%s",cJSON_Print(argjson));
	 }*/
	printf("\ndata=%s",buf);
       pclose(fp);
}


int main( )
{
	int argc;
	char **argv;	
	argc=3;
	argv=(char**)malloc(argc*sizeof(char*));
	for(int i=0;i<argc;i++)
			argv[i]=(char*)malloc(100*sizeof(char));
	

	strcpy(argv[0],"lightning-cli");
	strcpy(argv[1],"getinfo");
	argv[2]=NULL;

	make_command(argc-1,argv);	

     return 0;
}
