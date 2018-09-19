#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct region {
  char id[100];
  char type[10];
  char function_name[100];
  char address[10];
  char start[10];
  char end[10];
  char start_cpu[20];
  char end_cpu[20];
  char level[10];
  char nested[100];
};

struct single {
  char operation_id[100];
  char timestamp[50];
  char reference_address[10];
  char associated_function_call_instance_id[50];
};

int check_file(char id[100], FILE* lookup) {
  int count = 0;
  struct single read_single; 
  rewind(lookup);
  while (fscanf(lookup,"%s %s %s %s ", 
        read_single.operation_id, 
        read_single.timestamp, 
        read_single.reference_address, 
        read_single.associated_function_call_instance_id)==4) {

    if(strcmp(id, read_single.associated_function_call_instance_id) == 0) {
      count++; 
    }
    //return count;
  }
  return count;
}

void find_in_file(FILE* source, FILE* lookup) {
  char id[100];
  struct region single_region_line;
  while (fscanf(source,"%s %s %s %s %s %s %s %s %s %s ", 
        single_region_line.id,
        single_region_line.type,
        single_region_line.function_name,
        single_region_line.address,
        single_region_line.start,
        single_region_line.end,
        single_region_line.start_cpu,
        single_region_line.end_cpu,
        single_region_line.level,
        single_region_line.nested)==10) {

    if(single_region_line.id[0] <= '9' && single_region_line.id[0] >= '0') {
      printf("Startin loockup for: %s", single_region_line.id); 
      int count = check_file(single_region_line.id, lookup);
      printf(" -- Found Occurances: %d\n", count); 
    }else{
      printf("Ignoreing line for not being a number: %s\n", single_region_line.id); 
    }
  }
}

int main(int argc, char *argv[])
{
  FILE* region_file = fopen("region.out", "r");
  FILE* read_single_file = fopen("read_single.out", "r");
  if (read_single_file != NULL) {
    printf("file opened\n");
    find_in_file(region_file, read_single_file);
    /*

    struct single read_single; 
    struct single read_single_other; 

    while (fscanf(read_single_file,"%d %e %s %d ", 
          &read_single.operation_id, 
          &read_single.timestamp, 
          read_single.reference_address, 
          &read_single.associated_function_call_instance_id)==4) {
      printf("%s\n", read_single.reference_address); 
    }
    */
  } else {
    printf("file failed to open\n");
  }
}
