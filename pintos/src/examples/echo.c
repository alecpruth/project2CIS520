#include <stdio.h>
#include <syscall.h>


void prints(const char *s){
    char next_char;
    for(; *s; s++){
        next_char = *s;
        __asm__ __volatile__ \
        ("movl $0xff, %%eax\n\
        int $0x30"
        : /* No output registers */
        : "b" (next_char)
        : "%eax"
      );
    }
}
void exitp(void){
     __asm__ __volatile__ \
        (" movl $0x00, %eax\n\
        int $0x30"
      );
  }
    
int
main (int argc, char **argv)
{
  int i;
  prints("Hello World!\n");
  exitp();
  
  //cant use ASSERT here because it is user space and not Kernel space
  /*for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);
  printf ("\n");*/

  return EXIT_SUCCESS;
}
