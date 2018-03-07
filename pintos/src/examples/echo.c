#include <stdio.h>
#include <syscall.h>

    
int
main (int argc, char **argv)
{
  int i;
  char buffer[20];
  
  i = open("echo");
  read(i, buffer, sizeof(buffer)-1);
  
  printf("The fd return value is : %d\n", i);
  /*
  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);
  printf ("\n");
  */
  return EXIT_SUCCESS;
}
