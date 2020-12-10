/****************************************************************************
*                   KCW testing ext2 file system                            *
*****************************************************************************/


#include "type.h"




int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = p->gid = 0;
    p->cwd = 0;
    p->status = FREE;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }

}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0){
      iput(mip);
      }
  }
  exit(0);
}
// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "disk2";
int main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];
  char line[128], cmd[32], pathname[128];

  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }
  dev = fd;   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  inode_start = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->status = READY;
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|readlink]\n");
    printf("\t\t[open|close|pfd|lseek|read|write|cat|cp|quit]\ncmd: ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;
    sscanf(line, "%s %s", cmd, pathname);
    printf("cmd=%s pathname=%s\n", cmd, pathname);

    if (strcmp(cmd, "ls")==0)
       ls(pathname);
    if (strcmp(cmd, "cd")==0)
       cd(pathname);
    if (strcmp(cmd, "pwd")==0)
       pwd(running->cwd);
    if (strcmp(cmd, "mkdir")==0)
    	mymkdir(pathname);
    if (strcmp(cmd, "creat")==0)
    	creat_file(pathname);
    if (strcmp(cmd, "rmdir")==0)
    	myrmdir(pathname);
    if (strcmp(cmd, "link")==0)
    	mylink(line);
    if (strcmp(cmd, "unlink")==0)
    	my_unlink(pathname);
    if (strcmp(cmd, "symlink")==0)
    	my_symlink(line);
    if (strcmp(cmd, "readlink")==0)
      	printf("%s\n", my_readlink(pathname));
    if (strcmp(cmd, "open")==0)
    	open_file(line);
    if (strcmp(cmd, "close")==0)
    	close_file(atoi(pathname));
    if (strcmp(cmd, "pfd")==0)
    	pfd();
    if (strcmp(cmd, "lseek")==0)
    	my_lseek(line);
    if (strcmp(cmd, "read")==0)
    	myread_file(line);
    if (strcmp(cmd, "cat")==0)
    	mycat(line);
    if (strcmp(cmd, "write")==0)
    	write_file(line);
    if (strcmp(cmd, "cp")==0)
    	mycp(line);
    
    else if (strcmp(cmd, "quit")==0)
       quit();
    

  }
  return 0;
}


