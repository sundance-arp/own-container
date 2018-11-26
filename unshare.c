#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
  int flags = 0;
	int rc=0; 

  flags |= CLONE_NEWPID;
  flags |= CLONE_NEWNS;
  //flags |= CLONE_NEWUTS;
  //flags |= CLONE_NEWIPC;
  //flags |= CLONE_NEWNET;
	rc = unshare(flags);
  if(rc < 0){
    printf("unshare Error: %d\n", rc);
  }

	int pid = fork();
	switch(pid){
    // error
		case -1:
		  printf("fork Error: %d\n", pid);
			break;
    // child
		case 0:
			break;
    // parent
		default:
			{
				int status;
				waitpid(pid,&status,WUNTRACED);
				if(WIFEXITED(status)){
					printf("exit status = %d\n",WEXITSTATUS(status));
          return WEXITSTATUS(status);
				}else{
					printf("exit abnomally\n");
          return -1;
				}
			}
			break;
	}


	rc = chdir("./alpine-test");
  if(rc < 0){
    printf("chdir Error: %d\n", rc);
  }
	rc = chroot("./");
	if(rc < 0){
		printf("chroot Error: %d\n", rc);
		return(-1);
	}

  rc = mount("proc", "./proc", "proc", MS_NOSUID|MS_NOEXEC|MS_NODEV|MS_MGC_VAL, NULL);
	if(rc < 0){
		printf("proc mount Error: %d\n", rc);
		return(-1);
	}
	//rc = mount("devpts","/dev/pts", "devpts", MS_BIND, 0);
	//if(rc < 0){
	//    printf("devpts mount Error: %d\n", rc);
	//    return(-1);
	//}
	//mount -t devpts devpts /dev/pts
	//mount("devpts", "/dev/pts", "devpts",0,0);

	rc = execl("/bin/sh","/bin/sh", NULL);
	if(rc < 0){
		printf("exec Error: %d\n", rc);
		return(-1);
	}

}


