#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define SIZE 1024

typedef struct region {
  unsigned long id;
  char function_name[128];
} region;

typedef struct single {
  char type[8];
  char reference_address[16];
  unsigned long associated_function_call_instance_id;
} single;

typedef struct mapped_single_region {
  unsigned long id;
  char type[8];
  char reference_address[16];
  char function_name[128];
  int count;
} mapped_single_region;

typedef struct function_address {
  char type[8];
  char reference_address[16];
  char function_name[128];
  int count;
} function_address;

typedef struct function_address_grouping_struct {
  int id;
  mapped_single_region* mapped_single_region_data;
  int size;
  int finished;
  mapped_single_region* return_function_address;
} function_address_grouping_struct;

void write_out(mapped_single_region* groups, int size) {
  FILE *f = fopen("address_function_grouping.out", "w");
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }

  int i = 0;
  for(i = 0; i < size; i++) {
    fprintf(f, "<%s, %s, %s> : %d\n", groups[i].type, groups[i].reference_address, groups[i].function_name, groups[i].count);
  }

  fclose(f);
}

mapped_single_region* function_address_grouping(mapped_single_region* mapped_single_region_data, int* size) {
  mapped_single_region* function_address_data = malloc(*size * sizeof(*function_address_data));

  int index = 0;
  int i = 0;
  char current_function_name[100];
  char current_reference_address[11];
  char current_type[11];
  char check_function_name[100];
  char check_reference_address[11];
  char check_type[11];

  function_address_data[index].id = mapped_single_region_data[i].id;
  strcpy(function_address_data[index].type, mapped_single_region_data[i].type);
  strcpy(function_address_data[index].reference_address, mapped_single_region_data[i].reference_address);
  strcpy(function_address_data[index].function_name, mapped_single_region_data[i].function_name);
  function_address_data[index].count = mapped_single_region_data[i].count;

  for(i = 0; i < *size; i++) {
      strcpy(current_function_name, mapped_single_region_data[i].function_name);
      strcpy(current_reference_address, mapped_single_region_data[i].reference_address);
      strcpy(current_type, mapped_single_region_data[i].type);
      strcpy(check_function_name, function_address_data[index].function_name);
      strcpy(check_reference_address, function_address_data[index].reference_address);
      strcpy(check_type, function_address_data[index].type);

    if(strcmp(current_function_name, check_function_name) == 0 && strcmp(current_reference_address, check_reference_address) == 0 && strcmp(current_type, check_type) == 0) {
      function_address_data[index].count += 1;
    } else {
      index++;
      function_address_data[index].id = mapped_single_region_data[i].id;
      strcpy(function_address_data[index].type, mapped_single_region_data[i].type);
      strcpy(function_address_data[index].reference_address, mapped_single_region_data[i].reference_address);
      strcpy(function_address_data[index].function_name, mapped_single_region_data[i].function_name);
      function_address_data[index].count = mapped_single_region_data[i].count;
    }
  }

  function_address_data = realloc(function_address_data, index * sizeof(*function_address_data));
  *size = index;
  return function_address_data;
}

int compare_map(const void* a, const void* b) {
  mapped_single_region map_a = * ( (mapped_single_region*) a );
  mapped_single_region map_b = * ( (mapped_single_region*) b );

  //printf("%s :: %s\n", map_a.function_name, map_b.function_name);
  int diff = strcmp(map_a.function_name, map_b.function_name);
  
  if ( diff == 0 ) {
    //printf("%s :: %s\n", map_a.reference_address, map_b.reference_address);
    int diff2 = strcmp(map_a.reference_address, map_b.reference_address);
    if ( diff2 == 0 ) {
      //printf("%s :: %s\n", map_a.type, map_b.type);
      return strcmp(map_a.type, map_b.type);
    }
    return diff2;
  }

  return diff;
}

void* mapped_grouping(void* function_address_grouping_arg) {
  function_address_grouping_struct* data = (function_address_grouping_struct*) function_address_grouping_arg;
  //printf("%d : =================================\n", data->id);
  int* map_single_count = malloc(sizeof(int));
  *map_single_count = data->size;
  // BLOCK: Sort Single Region Map
  //printf("Sorting mapped data : %d\n", *map_single_count);
  qsort(data->mapped_single_region_data, *map_single_count, sizeof(mapped_single_region), compare_map);
  //printf("Map sorted\n");
  // BLOCK: Sort Single Region Map

  // BLOCK: Count Addresses And Functions
  int* function_address_grouping_count = malloc(sizeof(int));
  *function_address_grouping_count = *map_single_count;

  //printf("Grouping Functions\n");
  printf("Old Size %d :: ", *function_address_grouping_count);
  data->return_function_address = function_address_grouping(data->mapped_single_region_data, function_address_grouping_count);
  printf("New Size %d\n", *function_address_grouping_count);
  if(*function_address_grouping_count == data->size) {
    data->finished = 1;
  }
  data->size = *function_address_grouping_count;
  free(map_single_count);
  free(function_address_grouping_count);
  //printf("%d : =================================\n", data->id);
  pthread_exit(0);
}

mapped_single_region* map_single(region* sorted_region, single* sorted_single, int* region_size, int* single_size) {
  mapped_single_region* mapped_single_region_data = malloc(*single_size * sizeof(*mapped_single_region_data));

  int single_index = 0;
  int region_index = 0;
  int map_index = 0;
  unsigned long single_current = 0;
  unsigned long region_current = 0;
  int finished = 1;

  while(finished) {
    single_current = sorted_single[single_index].associated_function_call_instance_id;
    region_current = sorted_region[region_index].id;

    //printf("%ld(%d/%d) :: %ld(%d/%d)\n", single_current, single_index, *single_size, region_current, region_index, *region_size);

    if(single_current < region_current) {
      single_index++;
      if(single_index >= *single_size) {
        finished = 0;
      }
      continue;
    } else if ( single_current > region_current) {
      region_index++;
      if(region_index >= *region_size) {
        finished = 0;
      }
      continue;
    }
    mapped_single_region_data[map_index].id = single_current;
    strcpy(mapped_single_region_data[map_index].type, sorted_single[single_index].type);
    strcpy(mapped_single_region_data[map_index].reference_address, sorted_single[single_index].reference_address);
    strcpy(mapped_single_region_data[map_index].function_name, sorted_region[region_index].function_name);
    map_index++;
    single_index++;
    if(single_index >= *single_size) {
      finished = 0;
    }
  }
  mapped_single_region_data = realloc(mapped_single_region_data, map_index * sizeof(*mapped_single_region_data));
  *single_size = map_index;
  return mapped_single_region_data;
}

int compare_region(const void* a, const void* b) {
  region region_a = * ( (region*) a );
  region region_b = * ( (region*) b );
  unsigned long long_a = region_a.id;
  unsigned long long_b = region_b.id;
  
  if ( long_a == long_b ) {return 0;}
  else if ( long_a < long_b ) {return -1;}
  else {return 1;}
}

int compare_single(const void* a, const void* b) {
  single single_a = * ( (single*) a );
  single single_b = * ( (single*) b );
  unsigned long long_a = single_a.associated_function_call_instance_id;
  unsigned long long_b = single_b.associated_function_call_instance_id;

  if(long_a == 0 || long_b == 0) {
    printf("%s,%ld :: %s,%ld\n", single_a.reference_address, single_a.associated_function_call_instance_id, single_b.reference_address, single_b.associated_function_call_instance_id);
  }
  
  if ( long_a == long_b ) {return 0;}
  else if ( long_a < long_b ) {return -1;}
  else {return 1;}
}

void line_count(FILE* file, int* lines) {
  char buffer[SIZE + 1], lastchar = '\n';
  size_t bytes; *lines = 0;

  rewind(file);
  while ((bytes = fread(buffer, 1, sizeof(buffer) - 1, file))) {
    lastchar = buffer[bytes - 1];
    for (char *c = buffer; (c = memchr(c, '\n', bytes - (c - buffer))); c++) {
      *lines += 1;
    }
  }
  if (lastchar != '\n') {
    *lines += 1;
  }
}

region* load_data_region(FILE* region_file, int* size) {
  region* region_data = malloc(*size * sizeof(*region_data));
  char line[SIZE];
  int i = 0;
  const char split_token[] = " \t";
  int pass_header = 1;

  rewind(region_file);
  while (fgets(line,SIZE, region_file)) {
    if(pass_header) {
      pass_header = 0;
      continue;
    }
    line[strcspn(line, "\r\n")] = 0;
    if(strstr(line, "---")) {
      continue;
    }
    char *column = strtok(line, split_token);
    //printf("column: %s\n", column);
    region_data[i].id = strtoul(column, NULL, 10);
    //printf("id: %s, ", region_data[i].id);
    strtok(NULL, split_token); // Remove type as it is unneeded
    strcpy(region_data[i].function_name, strtok(NULL, split_token));
    //printf("function_name: %s\n", region_data[i].function_name);
    i++;
  }
  region_data = realloc(region_data, i * sizeof(*region_data));
  *size = i;
  return region_data;
}

single* load_data_single(FILE* single_file, char* type, int* size) {
  single* single_data = malloc(*size * sizeof(*single_data));
  char line[SIZE];
  int i = 0;
  const char split_token[] = " \t";

  rewind(single_file);
  while (fgets(line,SIZE, single_file)) {
    line[strcspn(line, "\r\n")] = 0;
    if(strstr(line, "---")) {
      continue;
    }
    char *column = strtok(line, split_token); // Remove id
    column = strtok(NULL, split_token); // Remove Timestamp

    column = strtok(NULL, split_token);
    strcpy(single_data[i].reference_address, column);
    //printf("id: %s, ", single_data[i].reference_address);

    column = strtok(NULL, split_token);
    single_data[i].associated_function_call_instance_id = strtoul(column, NULL, 10);
    //printf("call instance: %s\n", single_data[i].associated_function_call_instance_id);

    strcpy(single_data[i].type, type);
    i++;
  }

  single_data = realloc(single_data, i * sizeof(*single_data));
  *size = i;
  return single_data;
}

int main(int argc, char *argv[])
{
  clock_t start_time, time, result_time; 
  double time_taken;

  start_time = clock(); 
  time = clock();

  // BLOCK: Read File
  FILE* region_file = fopen("region.out", "r");
  FILE* read_single_file = fopen("read_single.out", "r");
  FILE* write_single_file = fopen("write_single.out", "r");
  // BLOCK: Read File

  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Read File %f Seconds\n", time_taken);

  if (read_single_file == NULL || write_single_file == NULL || region_file == NULL) {
    printf("One or more files could not be open\n");
    fclose(region_file);
    fclose(read_single_file);
    fclose(write_single_file);
    return 0;
  }

  printf("file opened\n");

  time = clock();
  // BLOCK: Line Count File
  int* region_file_count = malloc(sizeof(int));
  line_count(region_file, region_file_count);
  printf("Region file count: %d\n", *region_file_count);
  int* read_file_count = malloc(sizeof(int));
  line_count(read_single_file, read_file_count);
  printf("Read file count: %d\n", *read_file_count);
  int* write_file_count = malloc(sizeof(int));
  line_count(write_single_file, write_file_count);
  printf("Write file count: %d\n", *write_file_count);
  // BLOCK: Line Count File
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Line Count File %f Seconds\n", time_taken);

  time = clock();
  // BLOCK: File Processing
  printf("Processing region.out file\n");
  printf("Old Size %d :: ", *region_file_count);
  region* region_out_data = load_data_region(region_file, region_file_count);
  printf("New Size %d\n", *region_file_count);

  printf("Processing read_single.out file\n");
  printf("Old Size %d :: ", *read_file_count);
  single* read_out_data = load_data_single(read_single_file, "read", read_file_count);
  printf("New Size %d\n", *read_file_count);

  printf("Processing write_single.out file\n");
  printf("Old Size %d :: ", *write_file_count);
  single* write_out_data = load_data_single(write_single_file, "write", write_file_count);
  printf("New Size %d\n", *write_file_count);
  // BLOCK: File Processing
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: File Processing %f Seconds\n", time_taken);

  time = clock();
  // BLOCK: Single Out Data Combination
  int* single_count = malloc(sizeof(int));
  *single_count = (*read_file_count + *write_file_count);
  single* single_data = malloc(*single_count * sizeof(*single_data));

  printf("Putting read and write together in one array size %d (%d + %d)\n", *single_count, *read_file_count, *write_file_count);
  memcpy(single_data,                    read_out_data, *read_file_count * sizeof(*single_data));
  memcpy(single_data + *read_file_count, write_out_data, *write_file_count * sizeof(*single_data));
  // BLOCK: Single Out Data Combination
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Single Out Data Combination %f Seconds\n", time_taken);

  printf("Closing and Freeing Files and Variables\n");
  fclose(region_file);
  fclose(read_single_file);
  fclose(write_single_file);
  free(read_out_data);
  free(write_out_data);
  free(read_file_count);
  free(write_file_count);

  time = clock();
  // BLOCK: Sort Region And Single
  printf("Sorting region data\n");
  qsort(region_out_data, *region_file_count, sizeof(region), compare_region);
  printf("Sorting single data\n");
  qsort(single_data, *single_count, sizeof(single), compare_single);
  // BLOCK: Sort Region And Single
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Sort Region And Single %f Seconds\n", time_taken);

  time = clock();
  // BLOCK: Map Single to Region
  int* map_single_count = malloc(sizeof(int));
  *map_single_count = *single_count;

  printf("Mapping single and region data\n");
  printf("Old Size %d :: ", *map_single_count);
  mapped_single_region* mapped_single_region_data = map_single(region_out_data, single_data, region_file_count, map_single_count);
  printf("New Size %d\n", *map_single_count);
  // BLOCK: Map Single to Region
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Map Single to Region %f Seconds\n", time_taken);



  int thread_count = 1;
  pthread_t tid[5];
  int batch_count = *map_single_count / thread_count;

  int i = 0;
  int running = 1;
  while(running) {
    batch_count = *map_single_count / thread_count;
    function_address_grouping_struct* grouping_data = malloc(thread_count * sizeof(*grouping_data));
    for(i = 0; i < thread_count; i++) {

      int lower_bound = i*batch_count;
      int upper_bound = 0;
      if(i == thread_count-1) {
        upper_bound = *map_single_count-1;
      }else{
        upper_bound = ((i+1)*batch_count)-1;
      }
      grouping_data[i].size = upper_bound - lower_bound;

      grouping_data[i].mapped_single_region_data = malloc(grouping_data[i].size * sizeof(*grouping_data[i].mapped_single_region_data));
      int x = 0;
      for(x = lower_bound; x < upper_bound; x++) {
        grouping_data[i].mapped_single_region_data[x-lower_bound] = mapped_single_region_data[x];
      }

      grouping_data[i].finished = 0;
      grouping_data[i].id = i;
      printf("Lower Bound %d - Upper Bound: %d :: %d\n", lower_bound, upper_bound, grouping_data[i].size);
      //mapped_grouping((void*)&grouping_data[i]);
      pthread_create(&tid[i],NULL,mapped_grouping,(void*)&grouping_data[i]);
    }
    for(i = 0; i < thread_count; i++) {
      printf("Pthread join %d\n", i);
      pthread_join(tid[i], NULL);
    }
    int index = 0;
    running = 0;
    for(i = 0; i < thread_count; i++) {
      int x = 0;
      printf("%d SIZE: %d\n", i, grouping_data[i].size);
      for(x = 0; x < grouping_data[i].size; x++) {
        mapped_single_region_data[index] = grouping_data[i].return_function_address[x];
        index++;
      }
      if(!grouping_data[i].finished && thread_count != 1) {
        running = 1;
      }
    }

    printf("Old Size %d :: ", *map_single_count);
    *map_single_count = index;
    mapped_single_region_data = realloc(mapped_single_region_data, index * sizeof(*mapped_single_region_data));
    printf("New Size %d\n", *map_single_count);
    free(grouping_data);
    thread_count--;
  }






  time = clock();
  // BLOCK: Sort Single Region Map
  printf("Sorting mapped data\n");
  //qsort(mapped_single_region_data, *map_single_count, sizeof(mapped_single_region), compare_map);
  // BLOCK: Sort Single Region Map
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Sort Single Region Map %f Seconds\n", time_taken);

  time = clock();
  // BLOCK: Count Addresses And Functions
  int* function_address_grouping_count = malloc(sizeof(int));
  *function_address_grouping_count = *map_single_count;

  printf("Grouping Functions\n");
  printf("Old Size %d :: ", *function_address_grouping_count);
  //function_address* groups = function_address_grouping(mapped_single_region_data, function_address_grouping_count);
  printf("New Size %d\n", *function_address_grouping_count);
  // BLOCK: Count Addresses And Functions
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Count Addresses And Functions %f Seconds\n", time_taken);

  time = clock();
  // BLOCK: Write out to file
  write_out(mapped_single_region_data, *map_single_count);
  // BLOCK: Write out to file
  result_time = clock() - time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("TIME BLOCK: Write Out To File %f Seconds\n", time_taken);

  printf("Free data\n");
  free(region_out_data);
  free(single_data);
  free(mapped_single_region_data);
  //free(groups);
  
  free(region_file_count);
  free(single_count);
  free(map_single_count);
  free(function_address_grouping_count);

  result_time = clock() - start_time; 
  time_taken = ((double)result_time)/CLOCKS_PER_SEC; // in seconds 
  printf("Total execution time: %f Seconds\n", time_taken);
}
