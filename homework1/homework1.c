#include <stdio.h>
#include <string.h>


struct single {
  int operation_id;
  float timestamp;
  char reference_address[10];
  int associated_function_call_instance_id;
};

int main(int argc, char *argv[])
{
  FILE* read_single_file = fopen("read_single.out", "r");
  if (read_single_file != NULL) {
    printf("file opened\n");

    struct single read_single; 
    struct single read_single_other; 

    while (fscanf(read_single_file,"%d %e %s %d ", 
          &read_single.operation_id, 
          &read_single.timestamp, 
          read_single.reference_address, 
          &read_single.associated_function_call_instance_id)==4) {
      printf("%s\n", read_single.reference_address); 
    }
  } else {
    printf("file failed to open\n");
  }
}
