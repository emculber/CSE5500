#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SIZE 1024

typedef struct region {
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
} region;

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
  }
  return count;
}

int line_count(FILE* file) {
  char buffer[SIZE + 1], lastchar = '\n';
  size_t bytes;
  int lines = 0;

  while ((bytes = fread(buffer, 1, sizeof(buffer) - 1, file))) {
    lastchar = buffer[bytes - 1];
    for (char *c = buffer; (c = memchr(c, '\n', bytes - (c - buffer))); c++) {
      lines++;
    }
  }
  if (lastchar != '\n') {
    lines++;  /* Count the last line even if it lacks a newline */
  }

  return lines;
}

void find_in_file(FILE* source, FILE* lookup) {
  char id[100];
  region single_region_line;
  int continue_loop = 0;
  printf("Lines in file %d\n", line_count(source));
  return;
  while (continue_loop) {
      fscanf(source,"%s %s %s %s %s %s %s %s %s %s ", 
        single_region_line.id,
        single_region_line.type,
        single_region_line.function_name,
        single_region_line.address,
        single_region_line.start,
        single_region_line.end,
        single_region_line.start_cpu,
        single_region_line.end_cpu,
        single_region_line.level,
        single_region_line.nested);

    if(single_region_line.id[0] <= '9' && single_region_line.id[0] >= '0') {
      printf("Startin loockup for: %s", single_region_line.id); 
      int count = check_file(single_region_line.id, lookup);
      printf(" -- Found Occurances: %d\n", count); 
    }else{
      printf("Ignoreing line for not being a number: %s\n", single_region_line.id); 
    }
  }
}

region* load_data_region(FILE* region_file, int size) {
  region* region_data = malloc(size * sizeof(*region_data));
  char line[SIZE];
  int i = 0;

  rewind(region_file);
  while (fgets(line,SIZE, region_file)) {
    //printf("%s\n", line);
    strcpy(region_data[i++].id, strtok(line, " "));
  }
  return 0;
}

void load_data_single(FILE* single_file) {

}

int main(int argc, char *argv[])
{
  FILE* region_file = fopen("region.out", "r");
  FILE* read_single_file = fopen("read_single.out", "r");
  FILE* write_single_file = fopen("write_single.out", "r");
  if (read_single_file != NULL && write_single_file != NULL && region_file != NULL) {
    printf("file opened\n");
    //load_data(region_file, read_single_file, write_single_file);
    int region_file_count = line_count(region_file);
    load_data_region(region_file, region_file_count);
    //find_in_file(region_file, read_single_file);
  } else {
    printf("file failed to open\n");
  }
  fclose(region_file);
  fclose(read_single_file);
  fclose(write_single_file);
}
