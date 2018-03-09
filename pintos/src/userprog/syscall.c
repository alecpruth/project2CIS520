#include "userprog/syscall.h"
#include <stdio.h>
#include <debug.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
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
    unsigned chars_written = 0;
    
    fd = *(int *)(f->esp+4);
    buffer = (void *)*(uint32_t *)(f->esp+8);
    size = *(unsigned *)(f->esp+12);
    
    switch(fd){
        
     case STDOUT_FILENO:
        while(size--){
            printf("%c", *(char *)buffer++);
            chars_written++;
        }
        f->eax = chars_written;
        break;
        
     default:
        printf("Write system call!\n");
        break;
    }    
    
    //printf("File Descriptor: <%d> Pointer: <%x> Size: <%d> !\n", fd, (unsigned) buffer, size);
    //printf("Sys write");
}

static struct file * fd_to_file_ptr[20];

static void syscall_SYS_OPEN(struct intr_frame *f){
    
    static int fd = 3;
    struct file *file_ptr;
    
    char * filename = (char *)*(uint32_t *)(f->esp+4);
    file_ptr = filesys_open(filename);
    
    if( file_ptr != (struct file *)NULL) {
        f->eax = fd;
        fd_to_file_ptr[fd++] = file_ptr;
        /*
        char buf[20];
        file_read(file_ptr, buf, 19);
        printf("Content of file: <%s>\n", buf);
        */
    }
    else {
        printf("Error loading file: <%s>\n", filename);
        f->eax = -1;
    }
}

static void syscall_SYS_READ(struct intr_frame *f){
    
    int fd;
    struct file * file_ptr;
    char * buffer;
    unsigned size;
    
    fd = *(int *)(f->esp+4);
    buffer = (char *)*(uint32_t *)(f->esp+8);
    size = *(unsigned *)(f->esp+12);
    
    file_ptr = fd_to_file_ptr[fd];
    
    if(file_ptr == (struct file *)NULL)  {
        f->eax = -1;
        return;
    }
    
    /*
    char buf[20];
    file_read(file_ptr, buf, size);
    printf("Content of file: <%s>\nCharacters read: <%d>\n", buf, size);
    */
    
    /* Possible design choices:
     * 1. Only the owner of the file handle can access the file corresponding to that handle
     * 1. If the parent thread opened a new file, then, created a new child thread, the child thread inherits the parent's file handle
     * 1. In such cases, the child thread can access that file
     * 1.2. In any case, no two threads are allowed to write to the same file at once
     * 1.2. Need to implement locks to enforce exclusive write access
     */
    f->eax = file_read(file_ptr, buffer, size);   
}


static void syscall_SYS_CLOSE(struct intr_frame *f){
    
    int fd;
    struct file * file_ptr;
    
    fd = *(int *)(f->esp+4);    
    file_ptr = fd_to_file_ptr[fd];
    
    file_close(file_ptr);
    fd_to_file_ptr[fd] = (struct file *)NULL;
    
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
        
    case SYS_OPEN:
        syscall_SYS_OPEN(f);
        break;
        
    case SYS_READ:
        syscall_SYS_READ(f);
        break;
        
    case SYS_CLOSE:
        syscall_SYS_CLOSE(f);
        break;
      
    case SYS_EXIT:
        syscall_SYS_EXIT(f);
        break;
        
      default:
        printf("System Call <%d>!\n", f->eax);
        break;
    }
  
}
