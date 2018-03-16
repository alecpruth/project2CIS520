#include "userprog/syscall.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <debug.h>
#include "filesys/filesys.h"
#include "filesys/file.h"
#include <syscall-nr.h>
#include <list.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
bool valid_ptr(void *ptr);
bool valid_fd(int fd);
static struct file * fd_to_file_ptr[20];

bool valid_ptr(void * ptr)
{
    if( (ptr == NULL) || !is_user_vaddr(ptr) || !pagedir_get_page(thread_current()->pagedir, ptr) )
    {
        return false;
    }
    
    else {
        return true;
    }
}

bool valid_fd(int fd)
{
    if( (fd < 0) || (fd >= 20) || (fd_to_file_ptr[fd] == (struct file *)0)){
        return false;
    }
    else{
        return true;
    }
}

typedef int pid_t;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_SYS_EXIT(struct intr_frame *f);

static void syscall_SYS_WRITE(struct intr_frame *f){
    int fd;
    const void *buffer;
    unsigned size;
    unsigned chars_written = 0;
    struct file * file_ptr;
    
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
    
    case STDIN_FILENO:
        // terminate the process because it is trying to write to stdin
        // come up with a uniform implementation for all thread_exit()s
        f->eax = -1;
        break;
        
    case 2:
        while(size--){
            printf("%c", *(char *)buffer++);
            chars_written++;
        }
        f->eax = chars_written;
        break;
        
     default:
        if(!valid_fd(fd)){
            f->eax = -1;
            return;
        }
     
        file_ptr = fd_to_file_ptr[fd];
    
        if(file_ptr == (struct file *)NULL)  {
            f->eax = -1;
            return;
        }
        //printf("Write system call!\n");
        // To do: Validate the pointers
        f->eax = file_write(file_ptr, buffer, size);
        break;
    }    
    
    //printf("File Descriptor: <%d> Pointer: <%x> Size: <%d> !\n", fd, (unsigned) buffer, size);
    //printf("Sys write");
}

static void syscall_SYS_OPEN(struct intr_frame *f){
    
    static int fd = 3;
    struct file *file_ptr;
    
    char * filename = (char *)*(uint32_t *)(f->esp+4);
    
    if( !valid_ptr(filename) ) {
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }
    
    file_ptr = filesys_open(filename);
    
    
    if( file_ptr != (struct file *)NULL ) {
        f->eax = fd;
        fd_to_file_ptr[fd++] = file_ptr;
        /*
        char buf[20];
        file_read(file_ptr, buf, 19);
        printf("Content of file: <%s>\n", buf);
        */
    }
    else {
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
    
    /*if(!valid_fd(fd)){
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }*/
    
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
    
    /*if(!valid_fd(fd)){
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }*/
    
    file_ptr = fd_to_file_ptr[fd];
    
    file_close(file_ptr);
    fd_to_file_ptr[fd] = (struct file *)NULL;
    
}


static void syscall_SYS_EXIT(struct intr_frame *f){
        struct thread * curr_thread = thread_current();
        struct thread * next_waiting_thread;
        int status = *(int *)(f->esp+4);
        struct list_elem * next_elem;
        
        
         while(!list_empty(&curr_thread->waiters_list)) {
                //printf("sending status to waiter\n");
                next_elem = list_pop_front(&curr_thread->waiters_list);
                next_waiting_thread = list_entry(next_elem, struct thread, wait_elem);
                next_waiting_thread->child_exit_status = status;
                thread_unblock(next_waiting_thread);            
         }
        // To do: return the exit code to all the waiters
        
        // Get rid of the code that follows
        if(curr_thread->parent->waiting_for_child == true) {
            curr_thread->parent->child_exit_status = status;
            sema_up(&curr_thread->parent->wait_child_sema);
        }
        //printf("exiting\n");
        curr_thread->exit_status = status;
        thread_exit();
}    


static void syscall_SYS_TELL(struct intr_frame *f){
    
    int fd;
    struct file * file_ptr;
    
    fd = *(int *)(f->esp+4);  
    
    /*if(!valid_fd(fd)){
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }*/
      
    file_ptr = fd_to_file_ptr[fd];
    
    
    if(file_ptr == (struct file *)NULL)  {
        f->eax = -1;
        return;
    }
        
    f->eax= file_tell(file_ptr);
        
        
}    

static void syscall_SYS_SEEK(struct intr_frame *f){
       
    int fd;
    unsigned pos;
    struct file * file_ptr;
    
    fd = *(int *)(f->esp+4);  
    pos = *(unsigned *)(f->esp+8);  
    
    /*if(!valid_fd(fd)){
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }*/
    
    file_ptr = fd_to_file_ptr[fd];

    file_seek(file_ptr, pos);
        
}    

static void syscall_SYS_FILESIZE(struct intr_frame *f){
       
    int fd;
    struct file * file_ptr;
    
    fd = *(int *)(f->esp+4);   
    
    /*if(!valid_fd(fd)){
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }*/
     
    file_ptr = fd_to_file_ptr[fd];
    
    
    if(file_ptr == (struct file *)NULL)  {
        f->eax = -1;
        return;
    }
        
    f->eax= file_length(file_ptr);
        
        
}   

static void syscall_SYS_CREATE(struct intr_frame *f){
       
    unsigned init_size;
    
    
    const char * filename = (const char *)*(uint32_t *)(f->esp+4);
    init_size = *(unsigned *)(f->esp+8);
    
    if(filename == NULL || !valid_ptr(filename)){
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }
    
    else {
     f->eax = filesys_create(filename, init_size);
    }
        
        
}  

static void syscall_SYS_REMOVE(struct intr_frame *f){
       
    
    const char * filename = (const char *)*(uint32_t *)(f->esp+4);
    
    if(filename == NULL || !valid_ptr(filename)){
        //f->eax = false;
        *(uint32_t *)f->esp = SYS_EXIT;
        *(int *)(f->esp+4) = -1;
        syscall_SYS_EXIT(f);
        return;
    }
    
    else {
     f->eax = filesys_remove(filename);
    }
  
        
}    

static void syscall_SYS_WAIT(struct intr_frame *f){
       
    struct thread * curr_thread = thread_current();
    pid_t waitee_thread_pid = *(pid_t *)(f->esp+4);
    struct thread * waitee_thread = thread_list[waitee_thread_pid];
    
    // set curr_thread->waiting_for_child == true
    // We already have the id of the childthread specified as the parameter to wait
    // make a provision to wake the current thread up after the child thread exits
    // call thread_block() for this thread
    
    //If it is already in waitee list do not wait
    /*if(curr_thread->waitee_list == NULL){
        f->eax = -1;
        return;
    }
    struct list_elem trav = list_begin(&curr_thread->waitee_list);
    while(trav != list_tail(&curr_thread->waitee_list){
        if(trav == &waitee_thread->waitee_elem){
            f->eax = -1;
            return;
        }
        trav = trav->next;
    }
    list_push_back(&curr_thread->waitee_list, &waitee_thread->waitee_elem);*/
    
    list_push_back(&waitee_thread->waiters_list, &curr_thread->wait_elem);
    
    
    enum intr_level old_level;
    old_level = intr_disable();
    thread_block();
    intr_set_level(old_level);
    //printf("Waiting on child\n");
    f->eax = curr_thread->child_exit_status;

}   

static void syscall_SYS_EXEC(struct intr_frame *f){
        
        const char * cmd_line = (char *)*(uint32_t *)(f->esp+4);
        
        //note: need locks 
        //printf("About to execute\n");
        pid_t pid = (pid_t)process_execute(cmd_line);
        //printf("executed!\n");

        if(pid == TID_ERROR)  {
                f->eax = -1;
                return;
        }
        f->eax = pid;
        //printf("leaving exec\n");
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
        
    case SYS_SEEK:
        syscall_SYS_SEEK(f);
        break;
        
    case SYS_TELL:
        syscall_SYS_TELL(f);
        break;
    case SYS_FILESIZE:
        syscall_SYS_FILESIZE(f);
        break;
    case SYS_CLOSE:
        syscall_SYS_CLOSE(f);
        break;
    case SYS_CREATE:
        syscall_SYS_CREATE(f);
        break;
    case SYS_REMOVE:
        syscall_SYS_REMOVE(f);
        break;
    case SYS_WAIT:
        syscall_SYS_WAIT(f);
        break;
    case SYS_EXEC:
        syscall_SYS_EXEC(f);
        break;
    case SYS_EXIT:
        syscall_SYS_EXIT(f);
        break;
        
      default:
        printf("System Call <%d>!\n", f->eax);
        break;
    }
  
}
