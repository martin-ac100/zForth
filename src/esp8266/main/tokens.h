#define CMD_IS(c) !str_eq(cmdline,c)
#define CMD_N_IS(c,m) strN_eq(cmdline,c,m)
#define CMD_NEXT tokenGetNext(&cmdline, ' ')
#define SKIP_SPACES while(*cmdline == ' '){cmdline++;}

char * tokenGetNext( char **parsed_str, char delimin ); //pointer *parsed_str is modified!!
char  *skip_nZeros(const char *str, int n); // get (n+1)th NULL terminated string in *str 
int str_eq(const char *str1, const char *str2);  //returns 0 if str1 begins with str2 else -1
int strN_eq(char *str1, char *str2, int max); //returns int value N of string "str1N" or -1 if no match  
