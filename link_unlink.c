			/*************link***************/
#include "type.h"

int mylink(char *pathname){
  char path_temp[64], *temp, *srcLink, *newFile, *parent, *name;
  int oino, pino;
  MINODE *omip, *pmip;
  
  strcpy(path_temp, pathname);
  temp = strtok(path_temp, " ");
  srcLink = strtok(0, " ");
  newFile = strtok(0, " ");
  printf("srcLink = %s newFile = %s\n", srcLink, newFile);
  oino = getino(srcLink);
  if(oino) omip = iget(dev, oino);
  else 
  {
  	printf("old_file pathname %s does not exist\n", srcLink);
  	return 0;
  }
  if(!S_ISDIR(omip->INODE.i_mode))
  {
  	if(!getino(newFile)){
  		parent = dirname(newFile);
  		name = basename(newFile);
  		pino = getino(parent);
  		pmip = iget(dev, pino);
  		int ino = ialloc(dev);
  		int bno = balloc(dev);
  		
  		//omip->INODE.i_links_count++;
  
  		MINODE *mip = iget(dev, ino);
  		INODE *ip = &mip->INODE;
  		mip->ino = oino;
  		
  		ip->i_mode = 0x81A4; // 0100644: file type and permissions
  		ip->i_uid = running->uid; // owner uid
  		ip->i_gid = running->gid; // group Id
  		ip->i_size = 0; // size in bytes
  		//ip->i_links_count = 1;
  		ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  		ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  		ip->i_block[0] = bno; // new DIR has one data block
  		for(int i = 1; i < 15; i++){
  			ip->i_block[i] = 0;
  		}
 
  		enter_name(pmip, mip->ino, name, 0);
  		omip->INODE.i_links_count++;
  		mip->dirty = 1; // mark minode dirty
  		omip->dirty = 1;
  		pmip->dirty = 1;
  		iput(mip); // write INODE to disk
  		iput(omip);
  		iput(pmip);
  	
  	}
  	else
  	{
        	printf("the new_file %s already exists\n", newFile);
        	return 0;
  	}
  }
  else 
  {
  	printf("Error old_file can't be a directory\n");
  	return 0;
  }
}


int my_truncate(INODE *inode)
{
  int i;
  for(i = 0; i < 12; i++)
  {
  	inode->i_block[i] = 0;
  }
  return 1;
}

int my_unlink(char *pathname)
{
  char path_temp[64], buf[BLKSIZE], *parent, *name;
  int ino, pino, block, offset;
  MINODE *mip, *pmip;
  INODE *ip;
  
  strcpy(path_temp, pathname);
  ino = getino(path_temp);
  
  if(ino) mip = iget(dev, ino);		//if file name exists set mip
  else 
  {
  	printf("File name doesn't exist\n");
  	return 0;
  }
  
  if(!S_ISDIR(mip->INODE.i_mode))		//if file name isn't a directory
  {
  	parent = dirname(path_temp);
  	name = basename(pathname);
  	
  	printf("parent = %s file name = %s\n", parent, name);
  	
  	pino = getino(parent);
  	pmip = iget(dev, pino);
  	
  	rm_child(pmip, name);			//remove name from parent directory
  	
  	mip->INODE.i_links_count--;
  	if(mip->INODE.i_links_count > 0) mip->dirty = 1; //write INODE back to disk
  	else
  	{
  		printf("deallocating INODE'S data blocks\n");
  		
  		
  		ip = &mip->INODE;
  		
  		my_truncate(ip);
  		idalloc(dev, ino);
  	}
  	pmip->dirty = 1;
  	iput(mip);
  	iput(pmip);
  	printf("Unlink success\n");
  
  
  }
  else 
  {
  	printf("Error the file name given is a directory, can't unlink a directory\n");
   	return 0;
  }
  
  return 1;
}












