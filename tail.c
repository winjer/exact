/* $Id: tail.c,v 1.5 2003/01/24 13:59:45 doug Exp $
 *
 * These functions provide functionality very like that provided
 * in the perl File::Tail module.
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include "logger.h"

#define PAWS_MAX (1 * 1000000)

FILE *f;
unsigned long paws; // useconds
char *tail_buff;
unsigned int  tail_bufflen;

int tail_open(char *filename) {
	f=fopen(filename,"r");
	paws=PAWS_MAX;
	if(f) {
		fseek(f,0,SEEK_END);
		tail_buff=malloc(1024);
	}
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
	unsigned int lines;

	logger(LOG_DEBUG, "sleeping for %ld usecs\n", paws);
	usleep(paws);
	current=ftell(f);
	if(fseek(f,0,SEEK_END)==-1) {
		perror("exact");
	}
	end=ftell(f);
	if(fseek(f,current,SEEK_SET)==-1) {
		perror("exact");
	}
	tail_bufflen=end-current;
	logger(LOG_DEBUG,"%ld bytes added to file\n", tail_bufflen);
	if(tail_bufflen>0) {
		tail_buff=(char *)realloc(tail_buff,tail_bufflen+1);
	}
	if(!tail_buff) {
		logger(LOG_ERR,"unable to realloc %d bytes\n", tail_bufflen);
		exit(2);
	}
	read=fread(tail_buff,1,tail_bufflen,f);
	if(read!=tail_bufflen) {
		logger(LOG_ERR,"read %d bytes, wanted %d bytes\n", read, tail_bufflen);
		exit(2);
	}
	tail_buff[tail_bufflen]='\0'; // zero terminate it, so it can be matched
	lines=linecount(tail_buff,tail_bufflen);
	logger(LOG_DEBUG, "read %d lines with a pause of %ld usecs\n", lines, paws);
	if(lines>0) {
		// try and read one line per pause
		paws=paws/lines;
	} else {
		// exponential back off to a maximum
		paws=paws*2; 
		paws=paws > PAWS_MAX ? PAWS_MAX : paws;
	}
	return(tail_buff);
}
