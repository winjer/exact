/* $Id: conffile.h,v 1.2 2003/01/23 12:34:43 doug Exp $
*/

int conffile_read(char *filename);

char *conffile_param(char *name);
int conffile_param_int(char *name);
void conffile_check();
