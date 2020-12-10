		/***********************mkdir*************************/

#include "type.h"
	
		
int tst_bit(char *buf, int bit)
{
  // in Chapter 11.3.1
  return buf[bit/8] & (1 << (bit % 8));
}

int set_bit(char *buf, int bit)
{
  // in Chapter 11.3.1
  buf[bit/8] |= (1 << (bit % 8));
}

int decFreeInodes(int dev)
{
  //dec free inodes count in SUPER and GD
  char buf[BLKSIZE];
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);
  
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);


}

int decFreeBlocks(int dev)
{
  //dec free blocks count in SUPER and GD
  char buf[BLKSIZE];
  
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);
  
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
  
}

int incFreeInodes(int dev)
{
  char buf[BLKSIZE];
  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev)
{
  char buf[BLKSIZE];
  // inc free inodes count in SUPER and GD
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);

}
int clr_bit(char *buf, int bit) // clear bit in char buf[BLKSIZE]
{ 
  buf[bit/8] &= ~(1 << (bit%8)); 
}

int ialloc(int dev)  // allocate an inode number from inode_bitmap
{
  int i;
  char buf[BLKSIZE];
  // use imap, ninodes in mount table of dev
  get_block(dev, imap, buf);
  for (i=0; i<ninodes; i++){
	if (tst_bit(buf, i)==0){
		set_bit(buf, i);
		put_block(dev, imap, buf);
		// update free inode count in SUPER and GD
		decFreeInodes(dev);//possibly ignore for now?
		return (i+1);
	}
  }
  printf("Error in ialloc() no more free inodes\n");
  return 0; //out of FREE inodes
}

int balloc(int dev)
{
  int i;
  char buf[BLKSIZE];
  get_block(dev, bmap, buf);
  for(i = 0; i<nblocks; i++){
  	if(tst_bit(buf,i)==0){
  		set_bit(buf, i);
  		put_block(dev, bmap, buf);
  		decFreeBlocks(dev);
  		return i + 1;
  	}
  }
  printf("Error in balloc() no more data blocks\n");
  return 0;
}
 
int idalloc(int dev, int ino)
{
  
  char buf[BLKSIZE];
  if(ino > ninodes){
  	printf("inumber %d out of range\n", ino);
  	return 0;
  }
  //get inode bitmap block
  get_block(dev, imap, buf);
  clr_bit(buf, ino-1);
  //write buf back
  put_block(dev, imap, buf);
  //update free inode count in SUPER and GD
  incFreeInodes(dev);
}

int bdalloc(int dev, int bno)
{
  int i; 
  char buf[BLKSIZE];
  if(bno > nblocks){
  	printf("bnumber %d out of range\n", bno);
  	return 0;
  }
  get_block(dev, bmap, buf);
  clr_bit(buf, bno-1);
  //write buf back
  put_block(dev, bmap, buf);
  //update free block count in SUPER and GD
  //printf("got here\n");
  incFreeBlocks(dev);
  

}
int mymkdir(char *pathname){
  char *temp;
  int pino;
  MINODE *pmip = malloc(sizeof(MINODE));
  strcpy(temp, pathname);
  char *parent = dirname(temp);
  char *child = basename(pathname);
  printf("basename = %s\ndirname = %s\n", child, parent);
  pino = getino(parent);
  pmip = iget(dev, pino);
  if (S_ISDIR(pmip->INODE.i_mode)){ // check DIR type
  	//printf("dirname is a directory\n");
  	
  }
  else
  {
  	printf("error dirname: %s is not a directory\n", parent);
  	return 0;
  }
  
  if(!search(pmip, child)){
  	//printf("the directory does not exist yet\n");
  	if(kmkdir(pmip, child))return 1;
  }else{
  	printf("Error the directory already exisits\n");
  	return 0;
  }
  
  return 0;
  	

}

int kmkdir(MINODE *pmip, char *dir_name){
  int ino = ialloc(dev);
  int bno = balloc(dev);
  
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  
  ip->i_mode = 0x41ED; // 040755: DIR type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = BLKSIZE; // size in bytes
  ip->i_links_count = 2; // links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  ip->i_block[0] = bno; // new DIR has one data block
  for(int i = 1; i < 15; i++){
  	ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk
  
  // creating data block for new DIR containing . and .. entries
  char buf[BLKSIZE];
  DIR *dp = (DIR *)buf;
  char *cp = buf;
  
  //make . entry
  dp->inode = ino;
  dp->rec_len = 12;
  dp->name_len = 1;
  dp->name[0] = '.';
  
  //make .. entry
  cp += dp->rec_len;
  dp = (DIR *)cp;
  
  dp->inode = pmip->ino;
  dp->rec_len = BLKSIZE-12;		//rec_len spans block
  dp->name_len = 2;
  dp->name[0] = dp->name[1] = '.';
  put_block(dev, bno, buf);		//write to blk on disk
  
  
  
  enter_name(pmip, ino, dir_name, 1);
  

  return 1;
}

int enter_name(MINODE *pip, int ino, char *name, int option)
{
  char buf[BLKSIZE];
  DIR *dp;
  char *cp;
  int i;
  for(i = 0; i < 12; i++){
  	if(pip->INODE.i_block[i] == 0) break;
  	get_block(pip->dev, pip->INODE.i_block[i], buf);
  	
  	dp = (DIR *)buf;
  	cp = buf;
  	
  	while((cp + dp->rec_len) < buf + BLKSIZE){
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  	}
  	int remain = dp->rec_len - (4*((8 + dp->name_len + 3)/4));
  	int need_length = 4*((8 + strlen(name) + 3)/4);
  	
  	if (remain >= need_length){
  		//printf("remain = %d current dir length = %d\n", remain, need_length);
  		dp->rec_len = (4*((8 + dp->name_len + 3)/4));
  		cp += dp->rec_len;
  		dp = (DIR *)cp;
  		
  		dp->inode = ino;
  		dp->rec_len = remain;
  		dp->name_len = strlen(name);
  		
  		strncpy(dp->name, name, dp->name_len);
  		dp->name[strlen(name)] = '\n';
  		
  		
  		if(option){					//if the created file is directory
  			INODE *ip = &pip->INODE;		//update parents link count
  			ip->i_links_count++;
  			//printf("parent directory links = %d\n", ip->i_links_count);
  			pip->dirty = 1;
  			iput(pip);
  		}
  		put_block(pip->dev, pip->INODE.i_block[i], buf);
  		
  		return 1;
  	}
  	else{
  		printf("Not enough space in existing data blocks\n");
  	
  	
  	}

  }

}

int creat_file(char *pathname)
{
  char *temp;
  int pino;
  MINODE *pmip = malloc(sizeof(MINODE));
  strcpy(temp, pathname);
  char *parent = dirname(temp);
  char *child = basename(pathname);
  //printf("basename = %s\ndirname = %s\n", child, parent);
  pino = getino(parent);
  pmip = iget(dev, pino);
  if (S_ISDIR(pmip->INODE.i_mode)){ // check DIR type
  	//printf("dirname is a directory\n");
  	
  }else{
  	printf("error dirname: %s is not a directory\n", parent);
  	return 0;
  }
  
  if(!search(pmip, child)){
  	//printf("the directory does not exist yet\n");
  	if(my_creat(pmip, child))return 1;
  }else{
  	printf("Error the directory already exisits\n");
  	return 0;
  }
  
  return 0;


}

int my_creat(MINODE *pip, char *name)
{
  int ino = ialloc(dev);
  int bno = balloc(dev);
  
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  
  ip->i_mode = 0x81A4; // 0100644: file type and permissions
  ip->i_uid = running->uid; // owner uid
  ip->i_gid = running->gid; // group Id
  ip->i_size = 0; // size in bytes
  ip->i_links_count = 1; // links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
  ip->i_block[0] = bno; // new DIR has one data block
  for(int i = 1; i < 15; i++){
  	ip->i_block[i] = 0;
  }
  mip->dirty = 1; // mark minode dirty
  iput(mip); // write INODE to disk
  
  enter_name(pip, ino, name, 0);


}

int is_empty(MINODE *mip){
  char buf[BLKSIZE];
  get_block(dev, mip->INODE.i_block[0], buf);
  DIR *dp = (DIR*)buf;
  char *cp = buf;
  while(cp + dp->rec_len < buf + BLKSIZE){
	cp += dp->rec_len;
	dp = (DIR *)cp;
  }
  char lastName[32];
  strncpy(lastName, dp->name, dp->name_len);
  lastName[dp->name_len] = 0;
  if(strcmp(lastName, "..") == 0 || strcmp(lastName, "lost+found") == 0){
  	return 1;	//the directory really is empty
  }
  else{
  	printf("Directory is not empty the last file is %s\n", lastName);
  	return 0;
  }
  
  

}







