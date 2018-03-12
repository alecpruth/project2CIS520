#include <stdio.h>
#include <syscall.h>
#include <string.h>

    
int
main (int argc, char **argv)
{
  int fd;
  //bool createFile;
  unsigned initSize = 30;
  char buffer[20];
  int exitStatus;
  
  
  if(create("sample1.txt", initSize)){
      printf("File Created!!\n");
  }
  
  fd = open("sample1.txt");
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
  
  if((exitStatus = wait(fd)) == -1) {
      printf("Wait is not working!\n");
  }
  else {
      printf("wait is working!\n");
      printf("printf exit status returned from tell is <%d>\n", exitStatus);
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
