/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.int_watch.conf,v 1.2 2002/07/17 14:41:53 dts12 Exp $
 */
#ifndef SERVERUPTIME_H
#define SERVERUPTIME_H

/*
 * function declarations
 */
void            init_serverUptime(void);
Netsnmp_Node_Handler handler_serverUptime;
void format_tiempo(time_t, char*);
#endif                          /* SERVERUPTIME_H */
