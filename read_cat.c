			/*************** read cat ***************/
#include "type.h"

int myread_file(char *cmd_line)
{
  char *temp, *temp2, *fd_char, *bytes_char, buf[BLKSIZE];
  int fd, nbytes;
  
  temp = strtok(cmd_line, " ");
  fd_char = strtok(0, " ");
  bytes_char = strtok(0, " ");
  
  fd = atoi(fd_char);
  nbytes = atoi(bytes_char);
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
  if(running->fd[fd]->mode != 0 && running->fd[fd]->mode != 2)
  {
  	printf("file at fd index %d is not opened for READ 0 or READWRITE 2, mode = %d\n", fd,
  								running->fd[fd]->mode);
  	return 0; 
  }
  myread(fd, buf, nbytes);
  //printf("%s\n", buf);
  return 1;
}

int myread(int fd, char *buf, int nbytes)
{
  //printf("reading %d bytes from fd %d\n", nbytes, fd);
  int lbk, blk, startByte, remain, avil;	//logical block, physical block start byte, bytes 						remaining in lbk, avalible file size respectively
  OFT *oftp = (OFT *)malloc(sizeof(OFT));
  oftp = running->fd[fd];
  
  int count = 0;
  int cpy_size, indir_blk, indir_offset;
  avil = oftp->mptr->INODE.i_size - oftp->offset;	//available bytes in file
  char *cq = buf;					//cq points at buf[]
  char readbuf[BLKSIZE];
  int ibuf[256];
  
  while(nbytes && avil)
  {
  	lbk = oftp->offset / BLKSIZE;
  	startByte = oftp->offset % BLKSIZE;
  	
  	//printf("lbk = %d startByte = %d\n", lbk, startByte);
  	//printf("avil = %d, remain = %d, nbytes = %d\n",avil,remain,nbytes);
  	if(lbk < 12)	
  	{
  		//printf("-----direct blocks-----\n");
  		blk = oftp->mptr->INODE.i_block[lbk];
  	}
  	else if(lbk >= 12 && lbk < 256 + 12)
  	{
  		//printf("-----indirect blocks-----\n");
  		get_block(oftp->mptr->dev, oftp->mptr->INODE.i_block[12], (char*)ibuf);
  		blk = ibuf[lbk - 12];
  	}
  	else
  	{
  		//printf("-----double indirect blocks-----\n");
  		get_block(oftp->mptr->dev, oftp->mptr->INODE.i_block[13], (char*)ibuf);
  		indir_blk = (lbk - 256 - 12) / 256;
  		indir_offset = (lbk - 256 - 12) % 256;
  		blk = ibuf[indir_blk];
  		get_block(oftp->mptr->dev, blk, (char *)ibuf);
  		blk = ibuf[indir_offset];
  		
  	}
  	//printf("blk = %d\n", blk);
  	get_block(oftp->mptr->dev, blk, readbuf);	//get data block into readbuf
  	char *cp = readbuf + startByte;
  	remain = BLKSIZE - startByte;		//number of bytes remain in readbuf[]
  	
  	cpy_size = nbytes;
  	
  	if(cpy_size > avil)
  		cpy_size = avil;
  	
  	if(cpy_size < remain)
  	{
  		count += cpy_size;
  		oftp->offset += cpy_size;
  		strncpy(cq, cp, cpy_size - 1);
  		break;
  	}
  	else
  	{
  		count += remain;
  		oftp->offset += remain;
  		strncpy(cq, cp, remain - 1);
  		avil -= remain;
  		nbytes -= remain;
  	}
  	
  		
  	
  }
  //printf("\nmyread: read %d char from file descriptor %d\n", count, fd);
  return count;
}
int mycat(char *pathname)
{

  strcat(pathname, " 0");

	
  int fd = open_file(pathname);
  char mybuf[1024];
  int n;
	
  if(running->fd[fd] == NULL)
  {
	printf("error fd %d doesn't exist\n", fd);
	return 0;
  }
  if(running->fd[fd]->mode == 0) printf("file opened succesfully\n\n");
  while(n = myread(fd, mybuf, 1024)){
  	mybuf[n] = 0;
  	printf("%s", mybuf);
  }
  printf("\n");
  close_file(fd);
  return 1;
}





