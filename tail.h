/* $Id: tail.h,v 1.2 2003/01/22 17:19:10 doug Exp $
*/

extern char *tail_buff;
extern unsigned int tail_bufflen;

int tail_open(char *filename);
char *tail_read();
