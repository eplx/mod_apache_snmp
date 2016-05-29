/*
	Author: Esteban Pizzini - esteban.pizzini@gmail.com

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "ap_config.h"
#include "ap_mmn.h"
#include "httpd.h"
#include "http_main.h"
#include "http_config.h"
#include "http_connection.h"
#include "http_log.h"
#include "apr_buckets.h"
#include "apr_strings.h"
#include "util_filter.h"
#include "scoreboard.h"
#include "mpm_common.h"
#include "ap_mpm.h"
#include "ap_listen.h"
#include "scoreboard.h" // added to use ap_copy_scoreboard_process

//SNMP includes
#include "mod_ap2_snmp.h"		//header file for SNMP functions
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#ifndef DEFAULT_TIME_FORMAT
#define DEFAULT_TIME_FORMAT "%A, %d-%b-%Y %H:%M:%S %Z"
#endif

#define MOD_SNMP_DEBUG(message) ap_log_error(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, NULL, message);
//#define MOD_SNMP_DEBUG(message) ap_log_error(APLOG_MARK, APLOG_DEBUG, APR_SUCCESS, NULL,"mod-apache-snmp %s", message);
#define MOD_SNMP_ERROR(message) ap_log_error(APLOG_MARK, APLOG_ERR, APR_SUCCESS, NULL,"mod-apache-snmp %s", message);

module AP_MODULE_DECLARE_DATA ap2_snmp_module;

// module config variables (used in httpd.conf)
static char agent_address[LEN_BUFFER];
static char ap_snmp_community[LEN_BUFFER];
static char tmp_dir[LEN_BUFFER];
static char agentHttpAddress[LEN_BUFFER];
static char ap_snmp_community[LEN_BUFFER];
static int  ap_snmp_version = 0;
static char snmpv3_user[LEN_BUFFER];
static char snmpv3_auth[LEN_BUFFER];

// for post_config && ap2_snmp handler use
static char status_flags[SERVER_NUM_STATUS];
int server_limit, thread_limit;

/*
 	flag for avoid sending shutdown notification when "module_clean_up" is called.
	Shutdown notification is only send when Apache Shuts down.
*/
static int flag_init=0;

/*
 EXTRA Functions
 snmp_set: assign MIB values using Net-SNMP Libraries
*/
static int snmp_set(char* object, char type, char* value)
{
    int failures = 0;
    static int quiet = 0;

    netsnmp_session session, *ss;
    oid             name[MAX_OID_LEN];
    size_t          name_length;
    netsnmp_pdu    *pdu, *response = NULL;
    netsnmp_variable_list *vars;
    int             status;

    init_snmp("snmpapp");
    snmp_sess_init(&session);

    //session SNMP values
    session.retries = 1;
    session.timeout = 20;

    /*
    	Apache module SNMP configuration
    	(Use default values if there are not assigned in httpd.conf)
    */
    // COMMUNITY
    if (!strlen(ap_snmp_community))
    	session.community = DEFAULT_COMMUNITY;
    else
    	session.community = ap_snmp_community;

    session.community_len = strlen(session.community);

    // SNMP AGENT ADDRESS
    if (!strlen(agent_address))
        session.peername = DEFAULT_PEERNAME;
    else
    	session.peername = agent_address;

    // SNMP VERSION ( used by Apache module to communicate with the SNMP Agent)
    session.version = ap_snmp_version;

    if (session.version == 3)
    {
	    /* set SNMPv3 user name */
	    session.securityName = strdup(snmpv3_user);
	    session.securityNameLen = strlen(session.securityName);

	    /* set security level to authenticated, but not encrypted */
	    session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;

	    /* set the authentication method to MD5 */
	    session.securityAuthProto = usmHMACMD5AuthProtocol;
	    session.securityAuthProtoLen = sizeof(usmHMACMD5AuthProtocol)/sizeof(oid);
	    session.securityAuthKeyLen = USM_AUTH_KU_LEN;

	    if (generate_Ku(session.securityAuthProto,
	                    session.securityAuthProtoLen,
	                    (u_char *) snmpv3_auth, strlen(snmpv3_auth),
	                    session.securityAuthKey,
	                    &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
	        return 1;
	        }
    }

    // SNMP Agent sock (open connection)
    SOCK_STARTUP;
    ss = snmp_open(&session);

    /* ERROR */
    if (!ss) {
          MOD_SNMP_ERROR("AP2_SNMP: cannot connect with SNMP agent");
          MOD_SNMP_ERROR(agent_address);
        return 1;
    }

    if (ss == NULL)  SOCK_CLEANUP;

    pdu = snmp_pdu_create(SNMP_MSG_SET);
    name_length = MAX_OID_LEN;
    if (snmp_parse_oid(object, name, &name_length) == NULL)
    {
        snmp_perror(object);
        failures++;
    }
    else
    if (snmp_add_var(pdu, name, name_length, type, value))
    {
	snmp_perror(object);
	failures++;
    }

    /* if it fails, clean sock */
    if (failures) SOCK_CLEANUP;

    status = snmp_synch_response(ss, pdu, &response);

    if (response) snmp_free_pdu(response);

    /* close session */
    snmp_close(ss);
    SOCK_CLEANUP;

    /* if value assign fail */
    if (status != STAT_SUCCESS)
    {
        MOD_SNMP_ERROR("AP2_SNMP: Unable to set MIB value - Please ensure SNMP agent is running");
	MOD_SNMP_ERROR(object);
    	return 1; // with errors
    }
/* END SNMP set  function*/
return 0;
}
// End extra functions

/*
	set_agent_address: set Agent SNMP IP address if it was declared in httpd.conf
	(used for communication between module and agent snmp)
*/
static const char* set_agent_address(cmd_parms *cmd, void *dummy, const char* arg)
{
	if (strlen(arg)<=LEN_BUFFER)
		strcpy(agent_address,arg);
	else
	{	MOD_SNMP_ERROR("AP2_SNMP: Invalid snmp_agent_address");
		MOD_SNMP_ERROR(arg);
	}
	return NULL;
}

/*
	set_community: set SNMP community if it was declared in httpd.conf
	(used for communication between module and agent snmp)
*/
static const char* set_community(cmd_parms *cmd, void *dummy, const char* arg)
{
	if (strlen(arg)<=LEN_BUFFER)
		strcpy(ap_snmp_community,arg);
	else
	{	MOD_SNMP_ERROR("AP2_SNMP: Invalid snmp_community");
		MOD_SNMP_ERROR(arg);
	}
	return NULL;
}

/*
 	set_tmp_dir: temp directory for shared files between snmp agent and apache module.
 	(Apache must have write permission in this directory)
*/
static const char* set_tmp_dir(cmd_parms *cmd, void *dummy, const char* arg)
{
	if (strlen(arg)<=LEN_BUFFER)
	{
		strcpy(tmp_dir,arg);
		strcat(tmp_dir,TMP_FILENAME);

		// 12/12/2004 - it was moved to *ap2_snmp_pre_mpm* function
		//snmp_set("APACHE2-MIB::serverTmpDir.0",'s',tmp_dir);
	}
	else
	{	MOD_SNMP_ERROR("AP2_SNMP: tmp_dir exceeds buffer size");
		MOD_SNMP_ERROR(arg);
	}
	return NULL;
}

/*
 	set_snmp_version: SNMP version used by Apache module to communicate with the agent
*/
static const char* set_snmp_version(cmd_parms *cmd, void *dummy, const char* arg)
{
	    /*
	    #define SNMP_VERSION_1     0
	    #define SNMP_VERSION_2c    1
	    #define SNMP_VERSION_3     3
	    */
	switch(atoi(arg))
	{
		case 1:
		 ap_snmp_version = SNMP_VERSION_1;
		 break;
		case 2:
		 ap_snmp_version = SNMP_VERSION_2c;
		 break;
		case 3:
		 ap_snmp_version = SNMP_VERSION_3;
		 break;
		default:
		MOD_SNMP_ERROR("AP2_SNMP: Invalid SNMP version ")
	}
	return NULL;
}

/*
	set_snmpv3: if SNMPv3 is used, you must specify user and pass.
*/
static const char* set_snmpv3(cmd_parms *cmd, void *dummy, const char* user, const char* auth)
{
	if ((strlen(user)>LEN_BUFFER)||(strlen(auth)>LEN_BUFFER))
		MOD_SNMP_ERROR("AP2_SNMP: SNMPv3 - User or Password strings are too long.")
	else
	{
		strcpy(snmpv3_user,user);
		strcpy(snmpv3_auth,auth);
	}
	return NULL;
}

/*
 	set_snmp_http_address: IP Address and Port listened by Apache where Location /ap2_snmp/ is defined
 	(/ap2_snmp/ location is used by snmp agent to check Apache response)
*/
static const char* set_snmp_http_address(cmd_parms *cmd, void *dummy, const char* address, const char* port)
{

	if ((strlen(address)>LEN_BUFFER)||(strlen(port)>5))
		MOD_SNMP_ERROR("AP2_SNMP: Invalid Address or Port")
	else
	{
		strcpy(agentHttpAddress,address);
		strcat(agentHttpAddress,":");
		strcat(agentHttpAddress,port);

		/* for debugging only 
		MOD_SNMP_ERROR("AP2_SNMP: Setting agentHttpAddress to:");
		MOD_SNMP_ERROR(agentHttpAddress); 
		*/

	}
	return NULL;
}

/* Apache HOOK FUNCTIONS
 	module_clean_up: it\B4s call when main pool is clean up.
 	(in this hook, the module send shutdown notifications)
*/
static apr_status_t module_clean_up(void *arg)
{
	/* Apache is running --> send "shutting down" notification */
	if (flag_init)
	{
		MOD_SNMP_DEBUG("AP2_SNMP: module_clean_up (Ini)");
		// Apache ShutDown
		snmp_set("APACHE2-MIB::serverStatus.0",'i',"4");
		MOD_SNMP_DEBUG("AP2_SNMP: module_clean_up (End)");
		flag_init = 0;
	}
	return APR_SUCCESS;
}


/* PRE-MPM: called when Apache start/restart */
static int ap2_snmp_pre_mpm(apr_pool_t *p, ap_scoreboard_e sb_type)
{
 char* pidfile_name;
 ap_listen_rec* lr;
 char tmp_buffer[5];
 char oid_to_set[LEN_BUFFER];
 int total_ports=0, port_count = 0;

	/* check if tmp_dir conf directive was set. if not set default values.*/
	if (!strlen(tmp_dir)) {
		strcpy(tmp_dir,DEFAULT_TMP_DIR);
		strcat(tmp_dir,TMP_FILENAME);
	}

	/* check if agentHttpAddress conf directiva was set. if not set default value */
	if (!strlen(agentHttpAddress)) {
		strcpy(agentHttpAddress,DEFAULT_AGENT_HTTP_SERVER);
	}

	/* remove old tmp file */
	MOD_SNMP_DEBUG("AP2_SNMP: deleting old ap2_snmp"); //for debugging
	MOD_SNMP_DEBUG(tmp_dir);
	unlink(tmp_dir);

	/* sets configuration MIB values*/
	MOD_SNMP_DEBUG("AP2_SNMP: hook - pre_mpm (init)"); //for debugging
	snmp_set("APACHE2-MIB::serverTmpDir.0",'s',tmp_dir);
	snmp_set("APACHE2-MIB::agentHttpAddress.0",'s',agentHttpAddress);
	snmp_set("APACHE2-MIB::serverStatus.0",'i',"3");
	pidfile_name = ap_server_root_relative(p,ap_pid_fname);
	snmp_set("APACHE2-MIB::serverPidfile.0",'s',pidfile_name);
    	snmp_set("APACHE2-MIB::serverRestart.0",'s',ap_ht_time(p,ap_scoreboard_image->global->restart_time,DEFAULT_TIME_FORMAT, 0));
	MOD_SNMP_DEBUG("AP2_SNMP: hook - pre_mpm (end)");  // for debugging

	/* Assign serverListenPorts table (ports listened by apache) */
	for (lr = ap_listeners; lr; lr = lr->next) total_ports++;
	sprintf(tmp_buffer,"%d",total_ports);
	snmp_set("APACHE2-MIB::totalServerPorts.0",'i',tmp_buffer);

	for (lr = ap_listeners; lr; lr = lr->next)
	    {
		MOD_SNMP_DEBUG(lr->bind_addr->hostname);
		port_count++;
		sprintf(oid_to_set,"APACHE2-MIB::serverPortNumber.%d",port_count);
		sprintf(tmp_buffer,"%d",lr->bind_addr->port); // port --> tmp_buffer
		snmp_set(oid_to_set,'i',tmp_buffer);
	    	MOD_SNMP_DEBUG("AP2_SNMP: oid to set serverPortNumber");
		MOD_SNMP_DEBUG(oid_to_set);
	    	MOD_SNMP_DEBUG(tmp_buffer);
	    }
	flag_init = 1; 	// Apache started
return 0;
}

/*
 	ap2_snmp_post_config: called when apache start/restart after loading config files.
	(sets configuration MIB values)
*/
static int ap2_snmp_post_config(apr_pool_t *p,
			    	apr_pool_t *plog,
			    	apr_pool_t *ptemp,
			    	server_rec *s)
{
    char port_str[5];
    MOD_SNMP_DEBUG("AP2_SNMP: hook - post_config (init)");
    status_flags[SERVER_DEAD] = '.';
    status_flags[SERVER_READY] = '_';
    status_flags[SERVER_STARTING] = 'S';
    status_flags[SERVER_BUSY_READ] = 'R';
    status_flags[SERVER_BUSY_WRITE] = 'W';
    status_flags[SERVER_BUSY_KEEPALIVE] = 'K';
    status_flags[SERVER_BUSY_LOG] = 'L';
    status_flags[SERVER_BUSY_DNS] = 'D';
    status_flags[SERVER_CLOSING] = 'C';
    status_flags[SERVER_GRACEFUL] = 'G';
    status_flags[SERVER_IDLE_KILL] = 'I';
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_THREADS, &thread_limit);
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_DAEMONS, &server_limit);

    snmp_set("APACHE2-MIB::serverName.0",'s',s->server_hostname);
    /* register function to be called when main pool cleanup --> when apache is shutting down.*/
    apr_pool_cleanup_register(s->process->pconf, NULL, module_clean_up, apr_pool_cleanup_null);
    MOD_SNMP_DEBUG("AP2_SNMP: hook - post_config (end)");
    //raise(SIGSTOP); // for debug

    return OK;
}

/*
	ap2_snmp_log	(request_rec *r)
	For each request before logging sets mib values
*/
static int ap2_snmp_log	(request_rec *r)
{
	MOD_SNMP_DEBUG("AP2_SNMP: hook - log_transaction (init)");
	ap_log_error(APLOG_MARK,APLOG_DEBUG,0,r->server,"SNMP MODULE - hook_log_transaction") ;
	/*		check for HTTP errors	*/
	switch(r->status)
	{
	    case 400:
        	snmp_set("APACHE2-MIB::httpError400.0",'=',"1");
        	break;

	    case 403:
        	snmp_set("APACHE2-MIB::httpError403.0",'=',"1");
        	break;

	    case 404:
        	snmp_set("APACHE2-MIB::httpError404.0",'=',"1");
        	break;

	    case 405:
        	snmp_set("APACHE2-MIB::httpError405.0",'=',"1");
        	break;

	    case 500:
        	snmp_set("APACHE2-MIB::httpError500.0",'=',"1");
        	break;

	    case 501:
        	snmp_set("APACHE2-MIB::httpError501.0",'=',"1");
        	break;

	    case 505:
        	snmp_set("APACHE2-MIB::httpError505.0",'=',"1");
        	break;

	    default:
	    	break;

	}
	MOD_SNMP_DEBUG("AP2_SNMP: hook - log_transaction (end)");
}

/*
 	ap2_snmp_handler: handler for module requests (Location /ap2_snmp/ defined in httpd.conf)
 	function which writes ap2_snmp.tmp file with apache's information
*/
static int ap2_snmp_handler(request_rec *r)
{
process_score *ps_record;
worker_score *ws_record = apr_palloc(r->pool, sizeof *ws_record);
char *stat_buffer;
pid_t *pid_buffer;
int j, i, res;
unsigned long ready;
unsigned long busy;
unsigned long count;
unsigned long lres, my_lres, conn_lres;
apr_off_t bytes, my_bytes, conn_bytes;
apr_off_t bcount, kbcount;

apr_time_t nowtime;
apr_interval_time_t up_time;
float requests_per_sec, kbytes_per_sec, kbytes_per_request;

unsigned long kb_traffic;
char temporal[128];   			// response request --> OK
char access_buffer[LEN_BUFFER];		//for totalAccess
char traffic_buffer[LEN_BUFFER]; 	//for totalTraffic

char file_temp[LEN_BUFFER+20];
FILE *fp;		// file handler for interface file (snmp agent <--> apache module)

if (!r->handler || strcmp(r->handler, "ap2_snmp")) return (DECLINED);

ap_set_content_type(r, "text/html");
ap_rprintf(r,"OK");

// if we are checking only status --> do not write interface file
 if (!strcmp(r->uri,"/ap2_snmp/status"))
  {
  	MOD_SNMP_DEBUG("AP2_SNMP: hook - ap2_handler (end)");
  	return 0;
  }

pid_buffer = apr_palloc(r->pool, server_limit * sizeof(pid_t));
stat_buffer = apr_palloc(r->pool, server_limit * thread_limit * sizeof(char));

ready = 1;
busy = 1;
count = 0;
bcount = 0;
kbcount = 0;
kb_traffic = 0;

 	// view process and threads to get KB and access count

	//copied from mod_status.c
    for (i = 0; i < server_limit; ++i) {
        ps_record = ap_get_scoreboard_process(i);

        for (j = 0; j < thread_limit; ++j) {
            int indx = (i * thread_limit) + j;

            ap_copy_scoreboard_worker(ws_record, i, j);
            res = ws_record->status;

            lres = ws_record->access_count;
            bytes = ws_record->bytes_served;

				if (lres != 0 || (res != SERVER_READY && res != SERVER_DEAD)) {
	             count += lres;
                bcount += bytes;

                    if (bcount >= KBYTE) {
                        kbcount += (bcount >> 10);
                        bcount = bcount & 0x3ff;
                    }
                   
                }

			}
	}
// end copy from mod_status.c



/* Results --> Saved to interface file */
 kb_traffic = (unsigned long) kbcount;
 count++;	// add 1 to count the current request.

 // Get requests per second.
 nowtime = apr_time_now();	 // get now time..
 up_time = (apr_uint32_t) apr_time_sec(nowtime - ap_scoreboard_image->global->restart_time); //get up_time in seconds

 requests_per_sec = (float) count / (float) up_time;
 kbytes_per_sec = (float) kb_traffic / (float) up_time;
 kbytes_per_request = (float) kb_traffic / (float) count;

 // interface file
 if (!strlen(tmp_dir))
 {
  	strcpy(file_temp,DEFAULT_TMP_DIR);
 	strcat(file_temp,TMP_FILENAME);
 }
 else
    	strcpy(file_temp,tmp_dir);


 if ((fp = fopen(file_temp,"w")) == NULL)
	{
        MOD_SNMP_ERROR("AP2_SNMP: hook - ap2_handler: Unable to create temporary interface file.");
        MOD_SNMP_ERROR(file_temp);
	}
 else
 	{
	 	MOD_SNMP_DEBUG("AP2_SNMP: hook - ap2_handler: Writing interface file.");
	 	/* interface file format (binary):
	 		totalAccess 	(count)
	 		totalTraffic 	(kb_traffic)
	 		busyWorkers	(busy)
	 		idleWorkers	(idle)
	 	*/

	 	// only for debugging
	 	/*{
	 		char tmp[1000];
	 		sprintf(tmp,"AP2_SNMP: totalAccess value: %f",requests_per_sec);
	 		MOD_SNMP_ERROR(tmp);
sudo /usr/local/sbin/snmpd -c /etc/snmp/snmpd.con
	 	}
		*/
	 	fwrite(&count,sizeof(unsigned long),1,fp);			//totalaccess
	 	fwrite(&kb_traffic,sizeof(unsigned long),1,fp);		//traffic
	 	fwrite(&busy,sizeof(unsigned long),1,fp);						//busy workers
	 	fwrite(&ready,sizeof(unsigned long),1,fp);					//idle workers
	 	fwrite(&requests_per_sec,sizeof(float),1,fp);		//requests / second
	 	fwrite(&kbytes_per_sec,sizeof(float),1,fp);			//kbytes / second
	 	fwrite(&kbytes_per_request,sizeof(float),1,fp);		//kbytes / second
	 	fclose(fp);
 		MOD_SNMP_DEBUG("AP2_SNMP: hook - ap2_handler: Closing interface file.");
 	}

 MOD_SNMP_DEBUG("AP2_SNMP: hook - ap2_handler (end)");


 return OK;
}

// Register hooks.
static void register_hooks(apr_pool_t *p)
{
   ap_hook_handler(ap2_snmp_handler,NULL,NULL,APR_HOOK_MIDDLE);
   ap_hook_pre_mpm(ap2_snmp_pre_mpm,NULL,NULL,APR_HOOK_MIDDLE);
   ap_hook_post_config(ap2_snmp_post_config, NULL, NULL, APR_HOOK_MIDDLE);
   ap_hook_log_transaction(ap2_snmp_log, NULL, NULL, APR_HOOK_LAST);
}

// configuration directives.
static const command_rec ap2_snmp_cmds[] = {
    AP_INIT_TAKE1("snmp_agent_address", set_agent_address, NULL, RSRC_CONF,
                  "Agent Address"),
    AP_INIT_TAKE1("snmp_community", set_community, NULL, RSRC_CONF,
                  "Agent<->Apache Community"),
    AP_INIT_TAKE1("snmp_version", set_snmp_version, NULL, RSRC_CONF,
                  "Snmp version used by module"),
    AP_INIT_TAKE2("snmpv3_user", set_snmpv3, NULL, RSRC_CONF,
                  "Snmpv3 user and password"),
    AP_INIT_TAKE1("snmp_tmp_dir", set_tmp_dir, NULL, RSRC_CONF,
                  "Agent<->Apache Temp Dir"),
    AP_INIT_TAKE2("snmp_http_address", set_snmp_http_address, NULL, RSRC_CONF,
                  "IP address and port used by snmp agent to do http requests (for checking status)"),

    { NULL }
};

// Module structure
module AP_MODULE_DECLARE_DATA ap2_snmp_module = {
    STANDARD20_MODULE_STUFF,
    NULL,			/* create per-directory config structure */
    NULL,			/* merge per-directory config structures */
    NULL,			/* create per-server config structure */
    NULL,			/* merge per-server config structures */
    ap2_snmp_cmds,		/* command apr_table_t */
    register_hooks		/* register hooks */
};
