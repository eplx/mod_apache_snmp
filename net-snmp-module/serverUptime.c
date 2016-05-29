/*
	Copyright 2004 Esteban Pizzini

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/watcher.h>
#include "serverUptime.h"
#include "module_init.h"

/*
 * the variable we want to tie an OID to.  The agent will handle all
 * * GET and SET requests to this variable changing its value as needed.
 */

/* global variable for calculate time since last restart */
extern time_t tiempo_inicial;
extern char serverPidfile_str[MY_MAX_LEN];

static char serverUptime_str[MY_MAX_LEN];


// aux function which returns time in string format
void format_tiempo(time_t tsecs, char* buf_final)
{
    int days, hrs, mins, secs;
    char buf_temp[MY_MAX_LEN];

    secs = (int)(tsecs % 60);
    tsecs /= 60;
    mins = (int)(tsecs % 60);
    tsecs /= 60;
    hrs = (int)(tsecs % 24);
    days = (int)(tsecs / 24);

    strcpy(buf_final,"");
    if (days)
        {
        	sprintf(buf_temp, " %d day%s", days, days == 1 ? "" : "s");
        	strcat(buf_final,buf_temp);
        }
    if (hrs)
        {
 	       sprintf(buf_temp, " %d hour%s", hrs, hrs == 1 ? "" : "s");
 	       strcat(buf_final,buf_temp);
	}
    if (mins)
        {
               sprintf(buf_temp, " %d min%s", mins, mins == 1 ? "" : "s");
               strcat(buf_final,buf_temp);
	}
    if (secs)
        {
               sprintf(buf_temp, " %d sec%s", secs, secs == 1 ? "" : "s");
               strcat(buf_final,buf_temp);
	}

}


/*
 * our initialization routine, automatically called by the agent
 * (to get called, the function name must match init_FILENAME())
 */
void
init_serverUptime(void)
{
    static oid      serverUptime_oid[] =
        { MOD_SNMP_OID 2, 6 };

    /*
     * a debugging statement.  Run the agent with -DserverUptime to see
     * the output of this debugging statement.
     */
    DEBUGMSGTL(("serverUptime", "Initializing the serverUptime module\n"));


    /*
     * the line below registers our variables defined above as
     * accessible and makes them writable.  A read only version of any
     * of these registrations would merely call
     * register_read_only_int_instance() instead.  The functions
     * called below should be consistent with your MIB, however.
     *
     * If you wanted a callback when the value was retrieved or set
     * (even though the details of doing this are handled for you),
     * you could change the NULL pointer below to a valid handler
     * function.
     */

     netsnmp_register_watched_scalar(
        netsnmp_create_handler_registration(
            "serverUptime",
            handler_serverUptime,
            serverUptime_oid,
            OID_LENGTH(serverUptime_oid),
            HANDLER_CAN_RWRITE),
        netsnmp_create_watcher_info(
            &serverUptime_str, sizeof(serverUptime_str),
            ASN_OCTET_STR, WATCHER_MAX_SIZE)
        );

    DEBUGMSGTL(("serverUptime", "Done initalizing serverUptime module\n"));
}

int
handler_serverUptime(netsnmp_mib_handler *handler,
                    netsnmp_handler_registration *reginfo,
                    netsnmp_agent_request_info *reqinfo,
                    netsnmp_request_info *requests)
{
time_t tiempo_actual, tiempo_uptime;


    switch (reqinfo->mode) {

    case MODE_GET:
	if (check_estado())		// si Apache está ejecutándose..
	{
		tiempo_actual = time(NULL);
		tiempo_uptime = tiempo_actual - tiempo_inicial;
		format_tiempo(tiempo_uptime,serverUptime_str);
	}
	else
		strcpy(serverUptime_str,"Not running\r\n");

	snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR,
	                        (u_char *)
	                        serverUptime_str
	                        ,
	                        strlen(serverUptime_str)-1
	                        );

        break;
    case MODE_SET_RESERVE1:
	break;
    case MODE_SET_RESERVE2:
	break;
    case MODE_SET_FREE:
	break;
    case MODE_SET_ACTION:
	break;
    case MODE_SET_COMMIT:
        break;
    case MODE_SET_UNDO:
        break;

    default:
        /*
         * we should never get here, so this is a really bad error
         */
        return SNMP_ERR_GENERR;
    }
    return SNMP_ERR_NOERROR;
}

