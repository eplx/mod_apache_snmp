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
#include "serverListenPorts.h"
#include "module_init.h"


netsnmp_table_data_set *table_set;
netsnmp_table_row *row;

void
init_serverListenPorts(void)
{

    //int col1,col2;
    
    /*col1 = 1;
    col2 = 8080;*/
    
    oid my_registration_oid[] = {MOD_SNMP_OID 1, 10 };

    DEBUGMSGTL(("serverListenPorts","Init serverListenPorts\n")); 
 

    table_set = netsnmp_create_table_data_set("serverListenPorts");

    /*
     * allow the creation of new rows via SNMP SETs 
     */
    table_set->allow_creation = 1;

    /*
     * set up what a row "should" look like, starting with the index 
     */
    netsnmp_table_dataset_add_index(table_set, ASN_INTEGER);

   /*
     * define what the columns should look like.  both are octet strings here 
     */
    netsnmp_table_set_multi_add_default_row(table_set,
                                            2, ASN_INTEGER, 1,NULL, 0,
                                            0 /* done */ );
 
    /*
     * register the table 
     */
    /*
     * if we wanted to handle specific data in a specific way, or note
     * * when requests came in we could change the NULL below to a valid
     * * handler method in which we could over ride the default
     * * behaviour of the table_dataset helper 
     */
    netsnmp_register_table_data_set(netsnmp_create_handler_registration
                                    ("serverListenPorts", NULL,
                                     my_registration_oid,
                                     OID_LENGTH(my_registration_oid),
                                     HANDLER_CAN_RWRITE), table_set, NULL);

    /*
     * create the a row for the table, and add the data 
     */
    row = netsnmp_create_table_data_row();
    /*
     * set the index to the IETF WG name "snmpv3" 
     */
    /*netsnmp_table_row_add_index(row, ASN_INTEGER, &col1,
                                sizeof(col1));
*/

    /*
     * set column 2 to be the WG chair name "Russ Mundy" 
     */
//    netsnmp_set_row_column(row, 2, ASN_INTEGER, &col2, sizeof(col2));
//    netsnmp_mark_row_column_writable(row, 2, 1);        /* make writable via SETs */

    /*
     * add the row to the table 
     */
/*    netsnmp_table_dataset_add_row(table_set, row);*/

   
//    DEBUGMSGTL(("example_data_set", "Done initializing.\n"));
}


void clean_table()
{
   netsnmp_table_row *row;
   
   DEBUGMSGTL(("serverListenPorts","Clean table\n"));
    
    if (!table_set->table)
        return 0;

    for (row = table_set->table->first_row; row; row = row->next) 
    {
        netsnmp_table_dataset_remove_and_delete_row(table_set,row);
    }

}

