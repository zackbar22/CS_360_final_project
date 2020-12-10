		/***********rmdir rm_child**********/
		
#include "type.h"

int myrmdir(char *pathname)
{
  
  char temp[32], second_temp[32];
  int pino, ino;
  MINODE *mip = malloc(sizeof(MINODE));
  MINODE *pmip = malloc(sizeof(MINODE));
  
  strcpy(temp, pathname);
  strcpy(second_temp, pathname);
  
  char *path = dirname(temp);
  char *dir_name = basename(second_temp);	//name of directory that will be deleted
  printf("parent directory: %s\ndir_name: %s\n", path, dir_name);
  if(strcmp(pathname, ".")==0 || strcmp(pathname, "..")==0){
  	printf("Sorry, can't delete that directory\n");
  	return 0;
  }
  pino = getino(path);		//gets ino of parent directory
  pmip = iget(dev, pino);	//get's mip of parent 
  
  ino = getino(pathname);	//gets ino of directory to be deleted
  mip = iget(dev, ino);	//get's mip of selected directory
  
  
  
  if(ino){
  	//printf("Selected ino number = %d\n", ino);
  	if (S_ISDIR(mip->INODE.i_mode)){ //selected directory
  	
  		if((pmip->INODE.i_links_count > 2) && !is_empty(mip)){ //directory isn't empty
  			printf("Directory %s is not empty\n", dir_name); 
  			return 0;
  		} 
  		else {	//directory is empty and valid to be deleted 
  			for(int i = 0; i < 12; i++){
  				if(mip->INODE.i_block[i]==0){
  					continue;
  					
  				}
  				bdalloc(mip->dev, mip->INODE.i_block[i]);
  			}
  			idalloc(mip->dev, mip->ino);
  			iput(mip);
  			rm_child(pmip, dir_name);
  		
  		}
  	
  	} 
  	else {	//selected non directory
  		printf("%s is not a directory\n", dir_name);
  		return 0;
  	}
  } 
  else {
  	printf("pathname %s is not an existing directory\n", pathname);
  	return 0;
  }
  
}

int rm_child(MINODE *pip, char *dir_name)
{
  MINODE *last_entry = malloc(sizeof(MINODE));
  char buf[BLKSIZE];
  char lastName[32];
  get_block(dev, pip->INODE.i_block[0], buf);
  DIR *dp = (DIR*)buf;
  char *cp = buf;
  char *prev_cp = buf;
  DIR *prev = (DIR*)buf;
  int deleted_len;
  while(cp + dp->rec_len < buf + BLKSIZE){
  	prev = dp;
	cp += dp->rec_len;
	dp = (DIR *)cp;
	
  }
  
  strncpy(lastName, dp->name, dp->name_len);
  lastName[dp->name_len] = '\0';
  printf("lastName %s, dir_name %s\n", lastName, dir_name);
  if(strcmp(lastName, dir_name)==0){
  	printf("is last dir\n");
  	deleted_len = dp->rec_len;
  	prev->rec_len += dp->rec_len;
  	memcpy(dp, prev, dp->rec_len);
  	pip->INODE.i_links_count--;
  	pip->dirty = 1;
  	iput(pip);
  	put_block(fd, pip->INODE.i_block[0], buf);
  
  }
  else{
  	rmMore(pip, dir_name);
  }
}

int rmMore(MINODE *pip, char *dir_name){
  char buf[BLKSIZE];
  get_block(dev, pip->INODE.i_block[0], buf);
  printf("in rmMore with dir_name %s", dir_name);
  DIR *dp = (DIR*)buf;
  char temp[256];
  char *cp = buf;
  char *dp_char = buf;
  DIR *prev = (DIR*)cp;
  DIR *target = (DIR*)cp;
  int deleted_len, mem_size = 0;
  while(cp + dp->rec_len < buf + BLKSIZE){
  	printf("dp->name = %s\n", dp->name);
  	strncpy(temp, dp->name, dp->name_len);
  	temp[dp->name_len] = 0;
  	if(strcmp(temp, dir_name)==0){
		//target = (DIR *)cp;
		printf("found target name %s\n", dp->name);
		deleted_len = dp->rec_len;
		while(cp + dp->rec_len < buf + BLKSIZE){
			cp += dp->rec_len;
			mem_size += dp->rec_len;
			printf("mem_size = %d\n", mem_size);
			//memcpy(dp, cp, 12);
			dp = (DIR*)cp;
			
		}
		printf("final dp = %s\n",dp->name);
		dp->rec_len += deleted_len;
		printf("mem_size = %d\n", mem_size); 
		cp = dp_char + deleted_len;
		memcpy(dp_char, cp, mem_size);
		break;
			
	}
	else{
		cp += dp->rec_len;
		dp_char += dp->rec_len;
		dp = (DIR *)cp;
		//printf("dir name %s rec_len = %d\n", dp->name, dp->rec_len);
		//printf("looking for %s\n", dir_name);
	}
  }
  /*deleted_len += dp->rec_len;
  prev->rec_len = deleted_len;
  memcpy(dp, prev, dp->rec_len);*/
  pip->INODE.i_links_count--;
  pip->dirty = 1;
  iput(pip);
  put_block(fd, pip->INODE.i_block[0], buf);
  	//now target points to directory to be deleted 
  	//and dp points to last directory
}
