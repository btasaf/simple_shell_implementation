#include <signal.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <fcntl.h>
void set_line(char *line,int n);
void get_line(char *line);
int split_up(char* string, char* argv[]);
void fromfile(char *app[],char *file[]);
void tofile_append(char* app[],char *file[],const char *a_w);
void err(const char *mas);
int main()
{
	char line[1024],action[512],*action2[20],file[512],*file2[5],arg;
	int i,epflag=0,mypipe[2],pid,outcopy=dup(1),incopy=dup(0),rederect=0,len,*status;
	int pipeflag=0,errlognum;
	FILE *errors_log;
	
	
	

	if((errors_log=fopen("errors.log","a"))==NULL)
		err("fopen");
	
	errlognum=fileno(errors_log);

	if(dup2(errlognum,2)==-1)
		err("dup2");

	while(1)
	{	
		pipeflag=0;
		epflag=0;
		fflush(stdin);
		fflush(stdout);
		if(dup2(outcopy,1)==-1)
			err("dup2");
		if(dup2(incopy,0)==-1)
			err("dup2");	
		fflush(NULL);
		memset(line,'\0',sizeof(line));
		printf("$:");
		get_line(line);
		if(strncmp(line,"exit",4)==0)
		{
			printf("\ngood bye...\n");
			exit(0);
		}
		
		while(line[i]!='\0')
		{
			
			i=0;
			memset(action,'\0',sizeof(action));
			memset(file,'\0',sizeof(file));
			while(line[i]!='|'&&line[i]!='>'&&line[i]!='<'&&line[i]!='\0')
				i++;
			arg=line[i];
			strncpy(action,line,(const char)i);
			set_line(line,(i+1));
			int s=split_up(action,action2);
			action2[s]=(char *)NULL;
			i=0;
			


			if(action2[s-1][0]=='&'){
				epflag=1;
				action2[s-1][0]='\0';
				action2[s-1]=(char *)NULL;
			}
			
			if(arg=='\0')
			{
				
				
				memset(line,'\0',sizeof(line));
				if((pid=fork())<0)
					err("fork");
				if(pid==0)
				{
					execvp(action2[0],(char* const*)action2);
					err("execvp");
				}
				if(epflag==0){
					if(pipeflag==0)
						wait();
					else
					{
						wait();
						wait();
						pipeflag=0;
					}					
				}
				
				epflag=0;
				break;
				
			}
			if(arg=='|')
			{		
				pipeflag=1;		
				pipe(mypipe);
				if((pid=fork())<0)
					err("fork");
				if(pid==0)
				{
					close(mypipe[0]);
					if(dup2(mypipe[1],1)==-1)
						err("dup2");
					close(mypipe[1]);
					execvp(action2[0],(char* const*)action2);
					err("execvp");
				}
				else
				{
					close(mypipe[1]);
					if(dup2(mypipe[0],0)==-1)
						err("dup2");
					close(mypipe[0]);
					
					continue;
				}
			}
			if(arg=='>'||arg=='<')
			{
				if(arg=='>')
					if(line[0]=='>')
					{
						rederect=3;
						set_line(line,1);
						
					}
					else
						rederect=2;
				else
					rederect=1;

				len=strlen(line);
				strncpy(file,line,len);
				set_line(line,len);
				i=0;
				int s2=split_up(file,file2);	
				file2[s2]=(char*)NULL;
				
				if(file2[s2-1][0]=='&')
				{
					epflag=1;
					file2[s2-1]=NULL;
					
					if((pid=fork())==-1)
						err("fork");
					if(pid>0){
						continue;
					}
					setpgid(0,0);
				}			
				
				if(rederect==1)
				{
					fromfile(action2,file2);
				}
				if(rederect==2)
				{
					char temp='w';
					tofile_append(action2,file2,&temp);
				}
				if(rederect==3)
				{
					char temp='a';
					tofile_append(action2,file2,&temp);
				}
				
			}
			
			if(epflag==1){
				epflag=0;
				exit(0);							
			}
			
		}
		
	}
	return 0;
}
void set_line(char *line,int n)
{
	char *tem=line;
	char i=0,j=n;
	while(tem[i]!='\0')
	{
		line[i++]=tem[j++];
	}
	line[i]='\0';
}
void get_line(char *line){
	int i=0;
	char ch=getchar();
	while(ch!='\n'){
		line[i++]=ch;
		ch=getchar();
	}
	line[i++]='\0';
}
int split_up(char* string, char* argv[])
{
    char* p = string;
    int argc = 0;

    while(*p != '\0')
    {
        while(isspace(*p))
            ++p;

        if(*p != '\0')
            argv[argc++] = p;
        else
            break;

        while(*p != '\0' && !isspace(*p))
            p++;
        if(*p != '\0')
        {
            *p = '\0';
            ++p;
        }

    }

    return argc;
}
void tofile_append(char* app[],char *file[],const char *a_w){
	FILE *fd;
	int fdnum,copy=dup(1),pid;
	if((fd=fopen(file[0],(const char*)a_w))==NULL)
		err("fopen");
	
	fdnum=fileno(fd);
	if(dup2(fdnum,1)==-1)
		err("dup2");
	if((pid=fork())<0)
		err("fork");
	
	if(pid==0)
	{
		execvp(app[0],(char* const*)app);
		err("execvp");
	}
	wait();
	fflush(stdout);
	if(dup2(copy,1)==-1)
		err("dup2");

}
void fromfile(char *app[],char *file[]){
	FILE *fd;
	int fdnum,copy=dup(0),pid; 
	if(dup2(fdnum,0)==-1)
		err("dup2");
	if((fd=fopen(file[0],"r"))==NULL)
		err("fopen");	

	fdnum=fileno(fd);
	if(dup2(fdnum,0)==-1)
		err("dup2");

	if((pid=fork())==-1)
	err("fork");

	if(pid==0)
	{
	execvp(app[0],(char* const*)app);
	err("execvp");
	}
	wait();
	fflush(stdin);
	if(dup2(copy,0)==-1)
		err("dup2");
}
void err(const char *mas)
{
 perror(mas);
 exit(1);
}
