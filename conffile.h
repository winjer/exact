/* $Id: conffile.h,v 1.1 2003/01/22 18:18:49 doug Exp $
*/

int conffile_read(char *filename);

char *conffile_param(char *name);
int conffile_param_int(char *name);

