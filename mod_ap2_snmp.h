// Defines
#define LEN_BUFFER 254
#define KBYTE 1024
#define MBYTE 1048576L
#define GBYTE 1073741824L
#define STATUS_MAGIC_TYPE "application/x-httpd-status"

#define DEFAULT_COMMUNITY "public"
#define DEFAULT_PEERNAME "localhost"
#define DEFAULT_TMP_DIR "/tmp/"
#define TMP_FILENAME "ap2_snmp.tmp"
#define DEFAULT_AGENT_HTTP_SERVER "127.0.0.1:8080"

// Functions
static int snmp_set( char*, char, char*);
char* ap_get_server_version();


