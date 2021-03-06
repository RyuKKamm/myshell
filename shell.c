#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_LEN_LINE    30
#define LEN_HOSTNAME    30

/*int mkdir(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr,"디렉토리 생성\n");
        return 1;
    }
    if(mkdir(argv[1], 0755)){
        return 1;
    }
    return 0;
}미구현*/

//현재 위치를 프린트
int pwd(void){
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("현재 위치: %s\n", cwd);
}

//ls명령시 파일 타입을 읽음
void ls_type(struct stat buf){
    printf("%lo ",(unsigned long)buf.st_mode);
}

//ls명령을 구현
int ls(void){
    char *cwd = (char *)malloc(sizeof(char)*1024);
    DIR *dir = NULL;
    struct dirent *list = NULL;
    struct stat buf;
    getcwd(cwd, 1024);
	
    if((dir = opendir(cwd)) == NULL){
        printf("current directory error\n");
        exit(1);
    }
    while((list = readdir(dir)) != NULL){
	lstat(list->d_name, &buf);
	if(S_ISREG(buf.st_mode)){
	    printf("File ");
	}
	else if(S_ISDIR(buf.st_mode)){
	    printf("Directory ");
	}
	else{
	    printf("Unkown ");
	}
        printf("%s\n", list->d_name);
    }
    free(cwd);
    closedir(dir);
    return 0;
}


int main(void)
{
    char command[MAX_LEN_LINE];
    char *args[] = {command, NULL};
    int ret, status;
    pid_t pid, cpid;

    char hostname[LEN_HOSTNAME + 1];
    memset(hostname, 0x00, sizeof(hostname));
    gethostname(hostname, LEN_HOSTNAME);
    
    while (true){
        char *s;
	char *temp;
	char *tempcommand[MAX_LEN_LINE];
        int len, count = 0, i, j=0;
        printf("%s@%s$", getpwuid(getuid())->pw_name, hostname);
        s = fgets(command, MAX_LEN_LINE, stdin);
        if (s == NULL) {
            fprintf(stderr, "fgets failed\n");
            exit(1);
        }
	    
	//cd명령어를 입력한후 ls를 통해 하위 디렉토리를 확인후 원하는 디렉토리를 입력하면 그 디렉토리로 이동
	else if(strcmp("cd\n", command) == 0){
	    ls();
	    char dir[MAX_LEN_LINE];
	    scanf("%s", dir); 
	    char *movdir[]={dir, NULL};
	    printf("%s로 이동합니다\n\n", movdir[0]);
	    chdir(movdir[0]);
	    pwd();
	}
	    
        else{
            printf("\n명령어:%s\n/////////////////////\n", command);
	    temp = strtok(s," ;");
	    while(temp != NULL){
	        tempcommand[j] = temp;
                j++;
	        temp = strtok(NULL," ;");
	    }

	    for(i=0; i<j; i++){
	        s = tempcommand[i];
	        strcpy(command,s);
	        len = strlen(command);
	        if (command[len - 1] == '\n') {
	            command[len - 1] = '\0';
	        }
	        printf("%s\n", command);    
                  
		//exit 명령 실행
		if(strcmp("exit", command) == 0){
		    exit(1);
	        }
		    
		//ls 명령 실행    
	        if(strcmp("ls", command) == 0){   
	    	    ls();
	        }
		    
		//pwd명령 실행    
	        if(strcmp("pwd", command) == 0){
		    pwd();
	        }
    	        pid = fork();
	
                if (pid < 0) {
                    fprintf(stderr, "fork failed\n");
                    exit(1);
                } 
                if (pid != 0) {  /* parent */
                    cpid = waitpid(pid, &status, 0);
                    if (cpid != pid) {
                        fprintf(stderr, "waitpid failed\n");        
                    }     
		    printf("Child process terminated\n");
                    if (WIFEXITED(status)) {
                        printf("Exit status is %d\n", WEXITSTATUS(status)); 
                    }
                }
                else {  /* child */
	            ret = execve(args[0], args, NULL);
                    if (ret < 0) {   
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}
