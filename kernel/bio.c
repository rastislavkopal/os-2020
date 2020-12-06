// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct {
  struct spinlock lock;
  struct spinlock locks[NBUCKETS];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[NBUCKETS];
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  for(int i=0;i<NBUCKETS;i++){
    initlock(&bcache.locks[i],"bcache");
    // Create linked list of buffers
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[0].next;
    b->prev = &bcache.head[0];
    initsleeplock(&b->lock, "buffer");
    bcache.head[0].next->prev = b;
    bcache.head[0].next = b;
  }
}

// moves buf from previous bucket to "bucket"
// locks of both buckets must be locked
void
move_bucket(struct buf *b, int bucket)
{
  b->next->prev = b->prev;
  b->prev->next = b->next;
  b->next = bcache.head[bucket].next;
  b->prev = &bcache.head[bucket];
  bcache.head[bucket].next->prev = b;
  bcache.head[bucket].next = b;
}
// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int bucket = blockno % NBUCKETS;

  acquire(&bcache.locks[bucket]);

  // Is the block already cached?
  for(b = bcache.head[bucket].next; b != &bcache.head[bucket]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.locks[bucket]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  //release(&bcache.locks[bucket]);

  //acquire(&bcache.lock); // it is not necessary
  // TODO should use ticks instead to find LRU
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.buf; b != bcache.buf+NBUF; b = b+1){
    int current_bucket = b->blockno % NBUCKETS;
    if (bucket != current_bucket)
      acquire(&bcache.locks[current_bucket]);
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      if (current_bucket != bucket)
	move_bucket(b,bucket);
      if (bucket != current_bucket)
	release(&bcache.locks[current_bucket]);
      release(&bcache.locks[bucket]);
      //release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
    if (bucket != current_bucket)
      release(&bcache.locks[current_bucket]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");


  releasesleep(&b->lock);

  int bucket = b->blockno % NBUCKETS;

  acquire(&bcache.locks[bucket]);
  b->ticks = ticks;
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[bucket].next;
    b->prev = &bcache.head[bucket];
    bcache.head[bucket].next->prev = b;
    bcache.head[bucket].next = b;
  }
  
  release(&bcache.locks[bucket]);
}

void
bpin(struct buf *b) {
  int bucket = b->blockno % NBUCKETS;
  acquire(&bcache.locks[bucket]);
  b->refcnt++;
  release(&bcache.locks[bucket]);
}

void
bunpin(struct buf *b) {
  int bucket = b->blockno % NBUCKETS;
  acquire(&bcache.locks[bucket]);
  b->refcnt--;
  release(&bcache.locks[bucket]);
}


