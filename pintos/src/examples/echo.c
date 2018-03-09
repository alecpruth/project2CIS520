#include <stdio.h>
#include <syscall.h>
#include <string.h>

    
int
main (int argc, char **argv)
{
  int fd;
  char buffer[20];
  
  fd = open("sample.txt");
  printf("The fd return value is : %d\n", fd);
  
  switch( read(fd, buffer, sizeof(buffer)-1) )
  {
      case -1:
      printf("Error reading!\n");
      break;
      
      case 0:
      printf("Reached end of the file!\n");
      break;
      
      default:  
      printf("Buffer contains <%s>\n", buffer);
      break;
  }
  
  if(filesize(fd) == 19) {
      printf("Tell is working!\n");
  }
  else {
      printf("Tell is not working!\n");
      printf("printf filesize returned from tell is <%d>\n", filesize(fd));
  }
  
  
  //seek(fd, 19);
  
  memset(buffer, 0 , sizeof(buffer));
  //close(fd);
  
  switch( read(fd, buffer, sizeof(buffer)-1) )
  {
      case -1:
      printf("Error reading!\n");
      break;
      
      case 0:
      printf("Reached end of the file!\n");
      break;
      
      default:  
      printf("Buffer contains <%s>\n", buffer);
      break;
  }
  

  /*
  for (i = 0; i < argc; i++)
    printf ("%s ", argv[i]);
  printf ("\n");
  */
  return EXIT_SUCCESS;
}
