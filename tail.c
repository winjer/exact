/* $Id: tail.c,v 1.2 2003/01/22 12:04:21 doug Exp $
 *
 * These functions provide functionality very like that provided
 * in the perl File::Tail module.
*/


#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "debugmsg.h"

#define PAWS_MAX 10000000

FILE *f;
unsigned long paws; // useconds
char *buff;

int tail_open(char *filename) {
	f=fopen(filename,"r");
	paws=PAWS_MAX;
	return(f!=NULL);
}

int linecount(char *buff, int len) {
	int i;
	int ctr=0;
	for(i=0;i<len;++i) {
		if(buff[i]=='\n') ctr++;
	}
	return(ctr);
}

char *tail_read() {
	long current,end;
	size_t read;
	unsigned int len,lines;

	debugmsg(DMSG_USEFUL, "sleeping for %ld usecs\n", paws);
	usleep(paws);
	current=ftell(f);
	if(fseek(f,0,SEEK_END)==-1) {
		perror("exact");
	}
	end=ftell(f);
	if(fseek(f,current,SEEK_SET)==-1) {
		perror("exact");
	}
	len=end-current;
	debugmsg(DMSG_SYSTEM,"%ld bytes added to file\n", len);
	if(len>0) {
		buff=(char *)realloc(buff,len);
	}
	if(!buff) {
		debugmsg(DMSG_STANDARD,"unable to realloc %d bytes\n", len);
		exit(2);
	}
	read=fread(buff,1,len,f);
	if(read!=len) {
		debugmsg(DMSG_STANDARD,"read %d bytes, wanted %d bytes\n", read, len);
		exit(2);
	}
	lines=linecount(buff,len);
	debugmsg(DMSG_USEFUL, "read %d lines with a pause of %ld usecs\n", lines, paws);
	if(lines>0) {
		// try and read one line per pause
		paws=paws/lines;
	} else {
		// exponential back off to a maximum
		paws=paws*2; 
		paws=paws > PAWS_MAX ? PAWS_MAX : paws;
	}
	return(buff);
}
