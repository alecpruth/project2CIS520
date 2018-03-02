#include "userprog/syscall.h"
#include <stdio.h>
#include <debug.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static void syscall_SYS_WRITE(struct intr_frame *f){
    int fd;
    const void *buffer;
    unsigned size;
    fd = *(int *)(f->esp+4);
    buffer = (void *)*(uint32_t *)(f->esp+8);
    size = *(unsigned *)(f->esp+12);
    
    switch(fd){
        
     case STDOUT_FILENO:
        while(size--)
            printf("%c", *(char *)buffer++);
        break;
        
     default:
        printf("Write system call!\n");
        break;
    }    
    
    //printf("File Descriptor: <%d> Pointer: <%x> Size: <%d> !\n", fd, (unsigned) buffer, size);
    //printf("Sys write");
}

static void syscall_SYS_EXIT(struct intr_frame *f){
        struct thread * curr_thread = thread_current();
        int status = *(int *)(f->esp+4);
        
        if(curr_thread->parent->waiting_for_child == true) {
            curr_thread->parent->child_exit_status = status;
            sema_up(&curr_thread->parent->wait_child_sema);
            
        }
        thread_exit();
}    



static void
syscall_handler (struct intr_frame *f UNUSED) 
{
    
    uint32_t syscall_num;
    
    //printf("System Call: <%d> File Descriptor: <%d> Pointer: <%d> !\n", *(uint32_t *)f->esp);
    /*
    uint32_t * esp = (uint32_t *)(f->esp);
    char * buffer = (char *) *(esp+2);
    //printf("System Call: <%d> File Descriptor: <%d> Pointer: <%x> Size: <%d> !\n", *esp, *(esp+1), *(esp+2), *(esp+3));
    printf("%s\n", buffer);
    */
    
    syscall_num = *(uint64_t *)(f->esp);
    

    switch(syscall_num){
      case SYS_WRITE:
        syscall_SYS_WRITE(f);
        break;
      
      case SYS_EXIT:
        syscall_SYS_EXIT(f);
        break;
      
      default:
        printf("System Call <%d>!\n", f->eax);
        break;
    }
  
  
  //thread_exit ();
}
