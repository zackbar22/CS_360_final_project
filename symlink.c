				/**********symlink readlink*********/
#include "type.h"
int my_symlink(char *pathname)
{
  char ncmd[64], old_file[64], new_file[64], temp_pathname[128], buf[BLKSIZE];
  int oino, nino;
  MINODE *omip, *mip;
  INODE *ip;
  
  strcpy(temp_pathname, pathname);
  sscanf(temp_pathname, "%s %s %s", ncmd, old_file, new_file); 	//parses entire cmd line to get the old file and new file pathname
  
  printf("old_file = %s new_file = %s\n", old_file, new_file);
  
  oino = getino(old_file);
  if(oino) omip = iget(dev, oino);
  else
  {
  	printf("%s doesn't exist\n", old_file);
  	return 0;
  }
  if(!(S_ISDIR(omip->INODE.i_mode) || S_ISREG(omip->INODE.i_mode)))
  {
  	printf("%s is an invalid file type\n", old_file);
  	return 0;
  }
  printf("creating file\n");
  creat_file((char*)new_file);
  printf("post creat\n");
  nino = getino(new_file);
  if(nino)mip = iget(dev, nino);
  else
  {
  	printf("Error with creating or finding %s\n", new_file);
  	return 0;
  }
  

  
  ip = &mip->INODE;		//set ip to mip's inode
  ip->i_size = strlen(old_file) - 1;
  ip->i_mode = 0120000; 	//set new inode mode to link type
  ip->i_links_count++;
  omip->INODE.i_links_count++;
  printf("setting inode\n");
  strcpy((char*)mip->INODE.i_block, old_file);
  printf("inode set\n");
  mip->dirty = 1;
  omip->dirty = 1;
  iput(mip);
  iput(omip);
  //put_block(dev, block, buf);
  
  printf("symlink success\n");
  return 1;
}

char *my_readlink(char *pathname)
{
  char link[64], temp_pathname[64];
  int ino;
  MINODE *mip;
  INODE *ip;
  
  strcpy(temp_pathname, pathname);
  ino = getino(temp_pathname);
  
  if(ino) mip = iget(dev, ino);	//if pathname is an existing file set mip
  else
  {
  	printf("pathname %s doesn't exist\n", pathname);
  	return 0;
  }
  if(mip->INODE.i_mode == 0xA000)	//if file is symlink type
  {
  	return (char*)mip->INODE.i_block;
  }
  else
  {
  	printf("%s is not a symlink file\n", pathname);
  	return 0;
  }
  
}
