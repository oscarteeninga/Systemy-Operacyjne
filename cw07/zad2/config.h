#include <sys/types.h>
#include <sys/time.h>

#define PROJECT_ID 13
#define LOADER_INTERVAL 0
#define TRUCKER_INTERVAL 0

int parse_pos_int(char*);

struct timeval curr_time();
long int time_diff(struct timeval, struct timeval);
void print_time(struct timeval);
void print_current_time(void);

void die(char *msg);
