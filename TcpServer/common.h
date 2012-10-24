#define arraysize(x) sizeof(x)/sizeof(x[0])
#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif /*WIN32_LEAD_AND_MEAN */
#include <winsock2.h> /* IMPORTANT: Include winsock2.h before windows.h :-(*/
#include <windows.h>
#include <direct.h>   /* for _chdir-function */

/*------------------------------------------------------------------------
 * The following symbolic values/types are not defined in Windows
 *------------------------------------------------------------------------
 */

typedef int ssize_t;
#define SIGPIPE	13	/* Doesn't exist in windows */

/*------------------------------------------------------------------------
 * The following special symbols are different in Windows and Linux
 *------------------------------------------------------------------------
 */

#define SoeFileSeparator "\\"	/* Separator of filelabels */

/*------------------------------------------------------------------------
 * The following functions are different in Windows and Linux
 *------------------------------------------------------------------------
 */
#define chdir _chdir	/* change directory */

/*------------------------------------------------------------------------
 * To set the timeout in setsockopt Linux uses a struct timeval, Windows an int.
 * To develop portable code, use type soeTimeval to define the variable and
 * setSO_TIMEO(variable, value) to set the value (time in ms) of that variable.
 * setsockopt has then to be called as (for receive timeout):
 * setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (void *)&variable, sizeof(variable)).
 *------------------------------------------------------------------------
 */

#define setSO_TIMEO(x, y) x = y;
typedef int soeTimeval;

/*------------------------------------------------------------------------
 * With Windows x is in ms, with Linux in s. To be compatible, x should be
 * a multiple of 1000!
 *------------------------------------------------------------------------
 */
#define SoeSleep(x) Sleep((x))

/*------------------------------------------------------------------------
 * perror() cannot be used after winsock operations, so call
 * SoePerror(char *) instead.
 *------------------------------------------------------------------------
 */
#define SoeErrno WSAGetLastError()
char * SoeStrerror(DWORD errnum);
void SoePerror(char * lpszFunction);

/*------------------------------------------------------------------------
 * winsock-specific preamble.
 *------------------------------------------------------------------------
 */
int WSAinit(void);
#else /* WIN32 */
#ifndef __linux__
#error Fix environment: Define symbol WIN32 (for Windows) or __linux__ (for Linux)
#endif
/*------------------------------------------------------------------------
 * To make sure, everything is there.
 *------------------------------------------------------------------------
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
/*------------------------------------------------------------------------
 * The following symbolic values are not defined in Linux
 *------------------------------------------------------------------------
 */
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define WSAETIMEDOUT EAGAIN
#define WSAECONNABORTED ECONNABORTED

/*------------------------------------------------------------------------
 * The following special symbols are different in Windows and Linux
 *------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------
 * The following special symbols are different in Windows and Linux
 *------------------------------------------------------------------------
 */

#define SoeFileSeparator "/"	/* Separator of filelabels */

/*------------------------------------------------------------------------
 * The following functions are different in Windows and Linux
 *------------------------------------------------------------------------
 */

#define closesocket(s) close(s)
#define SoePerror(x) perror(x)
#define SoeStrerrorBufLen 64 /* Maximum string allowed for error descr. */
#define SoeErrno errno
#define _localtime64(x) localtime(x)
#define minTimestring 26

/*------------------------------------------------------------------------
/* To set the timeout in setsockopt Linux uses a struct timeval, Windows an int.
 * To develop portable code, see the explanation in the Linux part below.
 *------------------------------------------------------------------------
 */

#define setSO_TIMEO(x, y) x.tv_sec = (y)/1000; x.tv_usec = ((y) % 1000) * 1000;
#define SoeSleep(x) sleep((x)/1000)
typedef struct timeval soeTimeval;
typedef int SOCKET;
char * SoeStrerror(int errnum);
#endif /* WIN32 */

/*------------------------------------------------------------------------
/* Common to WIN32 and LINUX
 *------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

struct SoeListHdr {
	struct SoeListHdr	*l;
	struct SoeListHdr	*r;
};

extern unsigned short int portbase;

int errexit(const char *format, ...);
void printPage(FILE *f, char *line[], int numLines);
char *ascdate(char *buf, size_t bufSize);
int randomInt(int min, int max);
char *readLine( char *inbuf, int size_inbuf, FILE *stream);
SOCKET connectsock(const char *host, const char *service,
	const char *transport );
SOCKET passivesock(const char *service, const char *transport, int qlen);
SOCKET passiveTCP(const char *service, int qlen);
SOCKET passiveUDP(const char *service);
SOCKET connectTCP(const char *host, const char *service );
SOCKET connectUDP(const char *host, const char *service );
void *SoeListNodeNew(size_t size);
struct SoeListHdr *SoeListNew(void);
void SoeListNodeAdd(struct SoeListHdr * current, struct SoeListHdr *next);
void * SoeListNodeRemove(struct SoeListHdr * p);
int SoeListRemove(struct SoeListHdr * p);
void SoeListHdrPrint(struct SoeListHdr * p);
