#define BUT_COUNT 4
#define POW_COUNT 8
#define MAX_LONG_PRESS 4
#define BUT_IO config.Button_io_nums
#define POW_IO config.Power_io_nums
#define TOP_COUNT 7
#define FW_VERSION "0.51a"

char mac_str[18];

typedef struct {
	uint8_t mesh;
	uint8_t Button_io_nums[BUT_COUNT];
	uint8_t Power_io_nums[POW_COUNT];
	uint8_t Power_interlocks[POW_COUNT]; //bitmask of button's interlock group e.g. 0b00100010 = is in interlock grp 1 and 5
	uint8_t OW_io;
	uint32_t PWM_io[4];
	uint32_t PWM_duty[4];
	uint32_t PWM_period;
	uint8_t distance_trigger_io;
	uint8_t distance_echo_io;
	int click_delay;
	int long_press_times[MAX_LONG_PRESS];
	uint8_t button_pressed_value; // 1 if no PULL UP, 0 if there's a PULL UP
	char but_actions[256]; //"click\0002click\000lpress\0002lpress\000"; 4 zero ended strings for each button
	char sub_topics[192]; //"Topic1\000\Topic2\000Topic3\000"; 1 constant (MAC) + 6 custom topics zero ended
	uint8_t topic_actions[TOP_COUNT]; //index of sub_topic/ACTION -> index of commandline in but_actions[]
	uint8_t startup_action; // action index from but_actions that is run on startup sequence
	char pub_topic[32]; //1 Topic for publishing
} config_t;

uint8_t pwm_running;

typedef enum var_type {
	ui32_t = -3,
	i_t,
	ui8_t,
} var_type_t;

typedef struct {
	void *var;
	var_type_t type; // -3=uint32_t, -2=int, -1=uint_8, >= str with offset "type" times NULL char
	int str_count;
	char *value;
} param_t;
void update_msg_buffer(const char* pub_topic);
int cmdSet(char *cmdline);
int cmdGet(char *cmdline);
void printConfig();
