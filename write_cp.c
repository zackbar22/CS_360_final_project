				/********* write cp mv ********/
#include "type.h"

int write_file(char *pathname)
{
  char *fd_char, *txt_string, *temp, buf[BLKSIZE];
  int nbytes, fd;
  
  temp = strtok(pathname, " ");
  fd_char = strtok(0, " ");
  txt_string = strtok(0, " ");
  
  fd = atoi(fd_char);
  strcpy(buf, txt_string);
  nbytes = strlen(buf);
  
  
  
  return mywrite(fd, buf, nbytes);
}

int mywrite(int fd, char *buf, int nbytes)
{
  int lbk, startByte, cpy_size, indir_blk, indir_offset, blk, remain, count, ibuf[256];
  char wbuf[BLKSIZE];
  char *cp, *cq = buf;
  printf("fd = %d txt_string = %s nbytes = %d\n", fd, buf, nbytes);
  if(fd < 0 || fd >= NFD)
  {
  	printf("fd index %d is out of range\n", fd);
  	return 0;
  }
  if(running->fd[fd] == NULL)
  {
  	printf("Error running->fd[%d] doesn't exist\n", fd);
  	return 0;
  }
  if(!running->fd[fd]->mode)
  {
  	printf("file at fd index %d is opened for READ only\n", fd);
  	return 0; 
  }
  
  OFT *oftp = (OFT *)malloc(sizeof(OFT));
  oftp = running->fd[fd];
  MINODE *mip = oftp->mptr;
  
  while (nbytes > 0)
  {
  	lbk = oftp->offset / BLKSIZE;
  	startByte = oftp->offset % BLKSIZE;
  	
  	if (lbk < 12)
  	{
  		if(mip->INODE.i_block[lbk] == 0)
  		{
  			mip->INODE.i_block[lbk] = balloc(mip->dev);
  			printf("Allocating new block\n");
  		}
  		blk = mip->INODE.i_block[lbk];
  	}
  	else if(lbk >= 12 && lbk < 256 + 12)	//indirect blocks
  	{
  		printf("indirect blocks\n");
  		if(mip->INODE.i_block[12] == 0)
  		{
  			mip->INODE.i_block[12] = balloc(mip->dev);
  			//memset(ibuf, 0, 256);
  		}
  		get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
  		blk = ibuf[lbk - 12];
  		if(blk==0)
  		{
  			mip->INODE.i_block[lbk] = balloc(mip->dev);
  			ibuf[lbk - 12] = mip->INODE.i_block[lbk];
  			blk = ibuf[lbk - 12];
  		}
  	}
  	else
  	{
  		printf("double indirect blocks\n");
  		//memset(ibuf, 0, 256);
  		get_block(mip->dev, mip->INODE.i_block[13], (char *)ibuf);
  		indir_blk = (lbk - 256 - 12) / 256;
  		indir_offset = (lbk - 256 - 12) % 256;
  		blk = ibuf[indir_blk];
  		get_block(mip->dev, blk, (char *)ibuf);
  		blk = ibuf[indir_offset];
  	}
  	
  	memset(wbuf, 0, BLKSIZE);
  	get_block(mip->dev, blk, wbuf);
  	cp = wbuf + startByte;
  	remain = BLKSIZE - startByte;
  	
  	if(remain < nbytes)
  	{
  		strncpy(cp, cq, remain);
  		count += remain;
  		nbytes -= remain;
  		running->fd[fd]->offset += remain;
  		if(running->fd[fd]->offset > mip->INODE.i_size)
  			mip->INODE.i_size = running->fd[fd]->offset;
  		
  	}
  	else
  	{
  		strncpy(cp, cq, nbytes);
  		count += nbytes;
  		running->fd[fd]->offset += nbytes;
  		if(running->fd[fd]->offset > mip->INODE.i_size)
  			mip->INODE.i_size = running->fd[fd]->offset;
  		nbytes = 0;
  	}
  	put_block(mip->dev, blk, wbuf);
  	mip->dirty = 1;
  	printf("Wrote %d bytes to file\n", count);
  }
  return 1;
}

int mycp(char *pathname)
{
  char *src, *dest, *temp, buf[BLKSIZE], usable_dest[32];
  char src_open_str[32] = "open ";
  char dest_open_str[32] = "open ";
  char dest_open_cpy[32];
  int n, fd, gd;
  //OFT *fd = (OFT *)malloc(sizeof(OFT));
  //OFT *gd = (OFT *)malloc(sizeof(OFT));
  
  temp = strtok(pathname, " ");
  src = strtok(0, " ");
  dest = strtok(0, " ");
  
  printf("src = %s dest = %s\n", src, dest);

  strcat(src_open_str, src);
  strcat(src_open_str, " 0");
  
  strcat(dest_open_str, dest);
  strcat(dest_open_str, " 2");
  strcpy(dest_open_cpy, dest_open_str);
  fd = open_file(src_open_str);
  if(fd == -1)
  {
  	printf("src doesn't exist\n");
  	return 0;
  }
  gd = open_file(dest_open_cpy);
  if(gd == -1)
  {
  	printf("dest doesn't exist, creating file\n");
  	
  	strcpy(usable_dest, dest);
  	printf("%s %s\n", usable_dest, dest_open_str);
  	creat_file(usable_dest);
  	gd = open_file(dest_open_str);
  }
  
  while(n = myread(fd, buf, BLKSIZE)){
  	mywrite(gd, buf, n);
  }
  
  close_file(fd);
  close_file(gd);
  
  return 1;
}


