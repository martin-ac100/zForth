#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "esp_log.h"
static const char* TAG = "tokens.c";
char *tokenGetNext(char **p_buffer, char delim) {
	while (**p_buffer == ' ') *p_buffer += 1;
	char *next=*p_buffer;
        while (1) {
		if ( **p_buffer == delim ) {
			*p_buffer = *p_buffer+1;
			if ( delim && (**p_buffer == 0)) {*p_buffer = 0;} //if delim is not zero, then test double zero ended string = no more tokens in buffer left
			return next;
		}
		if ( **p_buffer == 0 ) {
			*p_buffer = 0;
			return next;
		}

		*p_buffer=*p_buffer+1;    
        }
}

char *skip_nZeros(const char *str, int n) {
	if (n == 0) {return str;} // n=0 -> return first string
	int i=0;
	for (int z=0; z < n; i++) {
		if (str[i] == 0) {z++;}
	}
	return (char *)(str+i);
}

int str_eq(const char *str1, const char *str2) { //returns 0 if str1 begins with str2 else -1
        int i=0;
	if ( str1 == NULL ) {return -1;};
	ESP_LOGD(TAG,"str_eq: %s %s\n",str1,str2);
        while (toupper(str1[i]) == toupper(str2[i])) {
                i++;
                if (str2[i] == 0) {
                        return 0;
                }
        }
        return -1;
}

int strN_eq(char *str1, char *str2, int max) {
	char N;
	//ESP_LOGD(TAG,"strN_eq %s <-->  %s :: %d\n",str1,str2,max);
	if (!str_eq(str1,str2)) {
		N = str1[strlen(str2)];
                if ( ('0' <= N) && (N < ('0'+max)) ) {
                        N -= '0';
			ESP_LOGD(TAG,"strN_eq returning N=%d\n",N);
			return (int)N;
                }
	}
	return -1;
}
