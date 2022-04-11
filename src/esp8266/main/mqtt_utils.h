#define PUBLISH(m) strcpy(payload_buffer,m);enow_send(msg_buffer)
#define PUBLISHF(args...) sprintf(payload_buffer,args);enow_send(msg_buffer)
#define MSG_PRINTF(args...) sprintf(payload_buffer+strlen(payload_buffer),args)
#define MSG_PRINT(m) strcat(payload_buffer,m)
int mqttGetCmdline(const char *rec, char **p_cmdline); //returns index of matching sub_topic and set p_cmdline to the cmdline start or return -1 if no match
char *getCmdlineCopy(const char *cmdline);
char msg_buffer[256];
char *payload_buffer;
