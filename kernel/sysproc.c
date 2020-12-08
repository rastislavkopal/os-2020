#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fcntl.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


uint64
sys_mmap(void)
{
  int len, prot, flags, fd;

  if(argint(1, &len) < 0)
    return -1;
  if(argint(2, &prot) < 0)
    return -1;
  if(argint(3, &flags) < 0)
    return -1;
  if(argint(4, &fd) < 0)
    return -1;

  uint64 addr = vma_map(myproc(), myproc()->ofile[fd], len, prot, flags);
  return addr;
}

uint64
sys_munmap(void)
{
  uint64 addr, end;
  int length, flag =1;
  struct mmap_vma * vma;
  struct proc * p = myproc();

  if(argaddr(0, &addr) < 0 || (argint(1, &length) < 0))
    return -1;
  if (length <=0)
    return -1;

  for (vma = &p->vma_list; vma < &p->vma_list + NVMA; vma++)
    if (addr >= vma->addr && addr < (vma->addr + vma->len)){
      flag = 0;
      break;
    }
  if (flag !=0)
    return -1;

  end = PGROUNDDOWN(addr+length);
  addr = PGROUNDUP(addr);
  length= end - addr;
  if (vma->flags == MAP_SHARED) // store modified
    filewrite(vma->file, addr, length);
  

  // unmmap
  uvmunmap(p->pagetable, addr, length/PGSIZE, 1);
  // update vma
  vma->len -= length;
  if (vma->addr == addr){
      if (vma->addr == p->vmastart)
	p->vmastart= addr + length;
      vma->addr = addr+length;
  }
  if (vma->len==0){
    fileclose(vma->file);
  }
  return 0;
}
