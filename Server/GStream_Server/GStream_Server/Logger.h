#ifndef _LOG
//define the output file name
#define LOGFILE "log"

//define log levels
#define _WARN "[WARN]"
#define _INFO "[INFO]"
#define _ERROR "[ERROR]"
#define _MAKE_IP(str) (str = TEXT("{"+str+"}"))
#define _MAKE_TAG(str) (str = TEXT("["+str+"]"))

//check if iostream was included
#include <iostream>
#include <time.h>

#define _LOG(msg,lvl){										\
	FILE* f=0;												\
	fopen_s(&f,LOGFILE,"a");								\
	if(f!=NULL){											\
		time_t raw;											\
		time(&raw);											\
		char* time = ctime(&raw);							\
		fwrite(time+11,8,1,f);								\
		fwrite(" ",1,1,f);									\
		fwrite(lvl,strlen(lvl),1,f);						\
		fwrite(": ",2,1,f);									\
		fwrite(msg,strlen(msg),1,f);						\
		fwrite("\n",1,1,f);									\
		fclose(f);											\
	}														\
}
#endif