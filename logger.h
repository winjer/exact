/* $Id: logger.h,v 1.1 2003/01/24 13:59:45 doug Exp $
*/

#include <stdarg.h>
#include <syslog.h>

void logger_init(int use_syslog, int debug);
void logger(int level, char *fmt, ...);
