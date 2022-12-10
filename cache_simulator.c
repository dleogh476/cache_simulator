#include <stdio.h>
#include <getopt.h>
#include <unistd.h> 
#include <math.h>
#include <stdlib.h>
#define LOAD 5
#define STORE 0
//./ab -s 2 -E 1 -b 4 -t hw04/gcc.trace <--컴파일 방법
// write-back and LRU 으로만 시작함
typedef struct cache_setting{
  int s; // set
  int S; // 2^s
  int b; // block
  int E; // set 속 block 개수
}Cache;

typedef struct Line {
  unsigned int valid;
  unsigned int tag;
  unsigned int lru;
}Line;

typedef struct result{
  unsigned int hits;
  unsigned int misses;
  unsigned int load_hits;
  unsigned int load_misses;
  unsigned int store_hits;
  unsigned int store_misses;
}Result;

typedef long unsigned int mem_addr;

Line **makeCache(int S, int E);
unsigned int getInfo(mem_addr a, unsigned int *set);
void checkCache(Line **my_cache, unsigned int set, unsigned int tag,int isLoad);
void update_lru(Line *set, int line);

//전역변수
Result re;
Cache cache;


int main(int argc, char** argv) {
  
  char *set_verify; 
  char *trace_file;
  char option;
//--------------------------------------------------------------------------
  //정상적인 입력이 되었는지 확인하는 변수
  int set_check = 0; 
  int line_check = 0; 
  int block_check = 0; 
  int trace_check = 0;
  
  while ((option = getopt(argc, argv, "s:E:b:t:h::")) != -1){
    switch(option){
    case 's':
      set_verify = optarg; // initialize var if its NULL	 
      cache.S = atoi(optarg);
      cache.s = (int)log2(cache.S); // 2의 s 제곱
      set_check = 1;
      break;
      
    case 'E':
      cache.E = atoi(optarg);
      line_check = 1;
      break;
	    
    case 'b':
      cache.b = atoi(optarg);
      block_check = 1;
      break;

    case 't':
      trace_file = optarg;
      trace_check = 1;
      break;
      
    case 'h':
      exit(0);
      
    default:
      exit(0);
    }
  }
  
  // 정상적으로 명령행 인자가 들어 갔는지 확인
  if ( set_check == 0 || line_check == 0 || block_check == 0 || trace_check == 0) {
    // 비정상적으로 인자가 들어옴
    exit(0);
  }
  
  // 정상적인 값의 인자가 들어왔는지 확인
  if (set_verify == NULL || cache.E == 0 || cache.b == 0 || trace_file == NULL) {
    // 잘못된값이 들어옴
    exit(0);
  }
//--------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------
  char c;
  
  //필요없는 값
  int d;
  
  //주소 값들어갈 타입 늘리기
  typedef long unsigned int mem_addr;
  mem_addr addr;
  
  unsigned int tag;
  unsigned int set;

  //출력하는 값들  
  re.hits = 0;
  re.misses = 0;
  re.load_hits =0;
  re.load_misses = 0;
  re.store_hits = 0;
  re.store_misses = 0;
  
  Line **my_cache = makeCache(cache.S, cache.E);
  FILE *trace = fopen(trace_file, "r"); // read in file
  
  int ret = fscanf(trace, " %c %lx,%d", &c, &addr, &d);

  while (ret != -1) {
    tag = getInfo(addr, &set);
    if(c == 'l') {
      checkCache(my_cache, set, tag, LOAD);
    }else if(c == 's'){
      checkCache(my_cache, set, tag, STORE);
    }
    ret = fscanf(trace, " %c %lx,%d", &c, &addr, &d);
  }
  fclose(trace);
  
  printf("total_hit : %d , total_miss : %d\n", re.hits, re.misses);
  printf("load_hit : %d , load_miss : %d\n", re.load_hits, re.load_misses);
  printf("store_hit : %d , store_miss : %d\n", re.store_hits, re.store_misses);
  return 0;
//--------------------------------------------------------------------------------------------
}


Line **makeCache(int S, int E) {
  Line** cache1 = malloc(sizeof(Line*)*S);
  for(int i = 0; i < S; i++) {
    cache1[i] = malloc(sizeof(Line)*E);
    for(int j = 0; j < E; j++) {
      cache1[i][j].valid = 0;
      cache1[i][j].lru = j;
    }
  }
  return cache1;
}


void checkCache(Line **my_cache, unsigned int set, unsigned int tag, int isLoad )  {
  printf("%d\n", isLoad);
  //hit
  for(int i = 0; i < cache.E; i++) {
    if(my_cache[set][i].valid != 0 && tag == my_cache[set][i].tag) {

      re.hits = re.hits + 1;
      //load, store 판별
      if(isLoad != 0){
	re.load_hits = re.load_hits + 1;
      }else{
	re.store_hits = re.store_hits + 1;
      }
      
      update_lru(my_cache[set], i);
      return;
    }
  }
  //miss
  for(int i = 0; i < cache.E; i++) {
    if(my_cache[set][i].lru == (cache.E - 1)) {
      //tag, valid 값 초기화
      my_cache[set][i].valid = 1;
      my_cache[set][i].tag = tag;
      
      update_lru(my_cache[set], i);

      re.misses = re.misses + 1;
      //load, store 판별
      if(isLoad != 0){
        re.load_misses = re.load_misses + 1;
      }else{
	re.store_misses = re.store_misses + 1;
      }
      return;
    }
  }
}


unsigned int getInfo(mem_addr a, unsigned int *set) {
  *set = (a<<(64 - (cache.s + cache.b)))>>(64 - cache.s);
  return a>>(cache.s + cache.b);
}


void update_lru(Line *set, int line) {
  for(int i = 0; i < cache.E; i++){
    if(i != line){
      if(set[i].lru < set[line].lru) set[i].lru = set[i].lru++;
    }
  }
  set[line].lru = 0;
}
