#include "common.h"

unsigned short int portbase = 0;
//static unsigned short int version = 5;

#ifdef WIN32

/*------------------------------------------------------------------------
 * Include the winsock library into the linker's path:
 * Project -> name of the project Properties -> Configuration Properties
 * -> Linker->Input->Additional Dependencies (a [...] button will
 * appear). Enter WS2_32.lib into that field and press [OK].
 *-----------------------------------------------------------------------
 */

/*------------------------------------------------------------------------
 * perror - print an error message
 *------------------------------------------------------------------------
 */

void SoePerror(char * lpszFunction) {
	// Retrieve the system error message for the last-error code

	char *msg;
	DWORD dw = GetLastError();
	if ((msg = SoeStrerror(GetLastError())) != NULL) {
		fprintf(stderr, "%s: %s\n", lpszFunction, msg);
		free(msg);
	} else fprintf(stderr, "%s: Unknown error\n", lpszFunction);
	return;
}

/*------------------------------------------------------------------------
 * SoeStrerror - Translate the error number to text
 * WARNING: The return value is allocated in dynamic memory,
 * so free the returned string when it is no longer needed.
 *------------------------------------------------------------------------
 */

char * SoeStrerror(DWORD errnum) {
	LPVOID lpMsgBuf;
	char *msg;
	DWORD msglen;

	if ((msglen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errnum,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) & lpMsgBuf,
			0, NULL)) == 0) return NULL;

	// Display the error message and exit the process
	msg = (char *) calloc(msglen + 1, 1);
	sprintf(msg, "%S", lpMsgBuf);
	LocalFree(lpMsgBuf);
	return msg;
}

/*------------------------------------------------------------------------
 * WSAinit - initialize the WINSOCK environment
 *------------------------------------------------------------------------
 */

int WSAinit(void) {
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
	int err = WSAStartup(wVersionRequested, &wsaData);

	/* Tell the user that we couldn't find a usable			*/
	/* WinSock DLL.											*/
	if (err != 0) {
		errexit("can't find a usable WINSOCK DLL;\nSocket error:  %d.\n",
			SoeErrno);
		return EXIT_FAILURE;
	}
	/* Confirm that the WinSock DLL supports 2.0.*/
	/* Note that if the DLL supports versions greater		*/
	/* than 2.0 in addition to 2.0, it will still return	*/
	/* 2.0 in wVersion since that is the version we			*/
	/* requested.											*/

	if (LOBYTE(wsaData.wVersion) != 2 ||
			HIBYTE(wsaData.wVersion) != 0) {
		/* Tell the user that we couldn't find a usable	*/
		/* WinSock DLL.									*/
		WSACleanup();
		errexit("can't find a usable WINSOCK DLL;\nSocket error:  %d.\n",
			SoeErrno);
		return EXIT_FAILURE;
	}

	/* The WinSock DLL is acceptable. Proceed.				*/
	/*** End of common preamble of every WINSOCK-Program  ***/
	return EXIT_SUCCESS;
}

/*------------------------------------------------------------------------
 * ascdate  -  convert internal date/time to ascii
 * including hours:mins:secs.
 * buf is a pointer to a caller's array of at least 26 octets
 *------------------------------------------------------------------------
 */
char *ascdate(char *buf, size_t bufSize) {
	char *p_buf;
	struct tm *newtime;
	time_t ltime;
	_time64(&ltime);
	newtime = _localtime64(&ltime);
	asctime_s(buf, bufSize, newtime);
	p_buf = strchr(buf, '\n');
	*p_buf = 0;
	return buf;
}

#else /* WIN32 */
#ifndef __linux__
#error Fix environment: Define symbol WIN32 for Windows or __linux__ for Linux.
#endif

/*------------------------------------------------------------------------
 * SoeStrerror - Translate the error number to text
 * WARNING: The return value is allocated in dynamic memory,
 * so free the returned string when it is no longer needed.
 *------------------------------------------------------------------------
 */

char * SoeStrerror(int errnum) {
	char *msg, *msgdyn;
	msgdyn = (char *) calloc(SoeStrerrorBufLen, 1);
#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) || _GNU_SOURCE
	/* XSI compliant version of strerror_r (see man strerror_r) */
	if (strerror_r(errnum, msgdyn, SoeStrerrorBufLen) < 0) {
		perror("strerror_r");
		msgdyn = "Unknown error";
		snprintf(msgdyn, SoeStrerrorBufLen,
			"Unknown error no. %d\n", errnum);
	}
#else
	msg = strerror_r(errnum, msgdyn, SoeStrerrorBufLen);
	strncpy(msgdyn, msg, SoeStrerrorBufLen);
#endif
	return msgdyn;
}

/*------------------------------------------------------------------------
 * WSAinit - No inizialisation of the WINSOCK environment in Linux
 *------------------------------------------------------------------------
 */

int WSAinit(void) {
	return EXIT_SUCCESS;
}

/*------------------------------------------------------------------------
 * ascdate  -  convert internal date/time to ascii
 * including hours:mins:secs.
 * buf is a pointer to a caller's array of at least 26 octets
 *------------------------------------------------------------------------
 */
char *ascdate(char *buf, size_t bufSize) {
	time_t now;
	if (bufSize < minTimestring) {
		memset(buf, '*', bufSize);
		buf[bufSize - 1] = 0;
	} else {
		time(&now);
		ctime_r(&now, buf);
		buf[strlen(buf) - 1] = '\0';
		return buf;
	}
}

#endif /* W32 */

/*------------------------------------------------------------------------
 * errexit - print an error message
 *------------------------------------------------------------------------
 */
int errexit(const char *format, ...) {
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	return 0;
}

/*------------------------------------------------------------------------
 * printPage - print an array of lines on file f
 *------------------------------------------------------------------------
 */
void printPage(FILE *f, char *line[], int numLines) {
	int i;
	for (i = 0; i < numLines; i++) fprintf(f, "%s", line[i]);
}

/*------------------------------------------------------------------------
 * randomInt returns a random value between min and max (both inclusive).
 * Function needs stdlib.h (0 <= rand() <= 32737 = RAND_MAX)
 * ------------------------------------------------------------------------
 */
int randomInt(int min, int max) {
	return (int) ((rand() / ((double) RAND_MAX + 1.0)) * (max + 1.0) + min);
}

/*------------------------------------------------------------------------
 * readLine - reads one line (buffered) until '\n' is found, or buffer
 * (inbuf) of size size_inbuf is full from file stream into buffer.
 * Return value (redundant) pointer to buffer.
 * ------------------------------------------------------------------------
 */

char *readLine(char *inbuf, int size_inbuf, FILE *stream) {
	/*  Read one line (buffered) until '\n' is found, or
	buffer (inbuf)
	of size size_inbuf is full from
	file stream
	into buffer.
	Return value (redundant) pointer to buffer. */
	char *p_inbuf;
	if (fgets(inbuf, size_inbuf, stream) == NULL) return NULL;
	if ((p_inbuf = strchr(inbuf, '\n')) != NULL) *p_inbuf = '\0';
	return inbuf;
}
/*------------------------------------------------------------------------
 * passiveTCP - create a passive socket for use in a TCP server
 *------------------------------------------------------------------------
 */

SOCKET passiveTCP(const char *service, int qlen) {
	/*
	 * Arguments:
	 *      service - service associated with the desired port
	 *      qlen    - maximum server request queue length
	 */
	return passivesock(service, "tcp", qlen);
}

/*------------------------------------------------------------------------
 * passiveUDP - create a passive socket for use in a UDP server
 *------------------------------------------------------------------------
 */
SOCKET passiveUDP(const char *service) {
	/*
	 * Arguments:
	 *      service - service associated with the desired port
	 */
	return passivesock(service, "udp", 0);
}

/*------------------------------------------------------------------------
 * connectTCP - connect to a specified TCP service on a specified host
 *------------------------------------------------------------------------
 */
SOCKET connectTCP(const char *host, const char *service) {
	/*
	 * Arguments:
	 *      host    - name of host to which connection is desired
	 *      service - service associated with the desired port
	 */
	return connectsock(host, service, "tcp");
}

/*------------------------------------------------------------------------
 * connectUDP - connect to a specified UDP service on a specified host
 *------------------------------------------------------------------------
 */
SOCKET connectUDP(const char *host, const char *service) {
	/*
	 * Arguments:
	 *      host    - name of host to which connection is desired
	 *      service - service associated with the desired port
	 */
	return connectsock(host, service, "udp");
}

/*------------------------------------------------------------------------
 * passivesock - allocate & bind a server socket using TCP or UDP
 *------------------------------------------------------------------------
 */
SOCKET passivesock(const char *service, const char *transport, int qlen) {
	/*
	 * Arguments:
	 *      service   - service associated with the desired port
	 *      transport - transport protocol to use ("tcp" or "udp")
	 *      qlen      - maximum server request queue length
	 */
	struct servent *pse; /* pointer to service information entry	*/
	struct protoent *ppe; /* pointer to protocol information entry*/
	struct sockaddr_in sin; /* an Internet endpoint address		*/
	int type; /* socket descriptor and socket type	*/
	SOCKET s;

	memset(&sin, 0, sizeof ( sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	/* Map service name to port number */
	if ((pse = getservbyname(service, transport)))
		sin.sin_port = htons(ntohs((unsigned short) pse->s_port)
			+ portbase);
	else {
		if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0) {
		errexit("can't get \"%s\" service entry;\nSocket error:  %d.\n", service, SoeErrno);
		return INVALID_SOCKET;
	}
	}
	/* Map protocol name to protocol number */
	if ((ppe = getprotobyname(transport)) == 0) {
		errexit("can't get \"%s\" protocol entry;\nSocket error:  %d.\n",
				transport, SoeErrno);
		return INVALID_SOCKET;
	}

	/* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	/* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s == INVALID_SOCKET) {
		errexit("can't create socket. Socket error:  %d.\n", SoeErrno);
		return INVALID_SOCKET;
	}

	/* Bind the socket */
	if (bind(s, (struct sockaddr *) &sin, sizeof ( sin)) < 0) {
		errexit("can't bind to %s port;\nSocket error:  %d.\n",
				service, SoeErrno);
		return INVALID_SOCKET;
	}
	if (type == SOCK_STREAM && listen(s, qlen) < 0)
		errexit("can't listen on %s port: %s;\nSocket error:  %d.\n",
			service, SoeErrno);
	return s;
}

/*------------------------------------------------------------------------
 * connectsock - allocate & connect a socket using TCP or UDP
 *------------------------------------------------------------------------
 */
SOCKET connectsock(const char *host, const char *service, const char *transport) {
	/*
	 * Arguments:
	 *      host      - name of host to which connection is desired
	 *      service   - service associated with the desired port
	 *      transport - name of transport protocol to use ("tcp" or "udp")
	 */
	struct hostent *phe; /* pointer to host information entry	*/
	struct servent *pse; /* pointer to service information entry	*/
	struct protoent *ppe; /* pointer to protocol information entry*/
	struct sockaddr_in sin; /* an Internet endpoint address		*/
	int type; /* socket descriptor and socket type	*/
	SOCKET s;

	memset(&sin, 0, sizeof ( sin));
	sin.sin_family = AF_INET;

	/* Map service name to port number */
	/* The services are defined in %SystemRoot\system32\drivers\etc\services */
	if ((pse = getservbyname(service, transport)))
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port = htons((unsigned short) atoi(service))) == 0) {
		errexit("can't get \"%s\" service entry;\nSocket error:  %d.\n",
				service, SoeErrno);
		return INVALID_SOCKET;
	}

	/* Map host name to IP address, allowing for dotted decimal */
	if ((phe = gethostbyname(host)))
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
		errexit("can't get \"%s\" host entry;\nSocket error:  %d.\n",
				host, SoeErrno);
		return INVALID_SOCKET;
	}


	/* Map transport protocol name to protocol number */
	if ((ppe = getprotobyname(transport)) == 0) {
		errexit("can't get \"%s\" protocol entry;\nSocket error:  %d.\n",
				transport, SoeErrno);
		return INVALID_SOCKET;
	}

	/* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	/* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s == INVALID_SOCKET) {
		errexit("can't create socket;\nSocket error:  %d.\n", SoeErrno);
		return INVALID_SOCKET;
	}

	/* Connect the socket */
/*------------------------------------------------------------------------
 * Generally, connection-based protocol sockets may successfully connect()
 * only once; connectionless protocol sockets may use  connect()  multiple
 * times to change their association.  Connectionless sockets may dissolve
 * the association by connecting to an address with the  sa_family  member
 * of sockaddr set to AF_UNSPEC (supported on Linux since kernel 2.2).
/*------------------------------------------------------------------------
*/
	if (connect(s, (struct sockaddr *) &sin, sizeof ( sin)) < 0)
		errexit("can't connect to %s.%s:\nSocket error:  %d.\n",
			host, service, SoeErrno);
	return s;
}

/*****************************************************
 *  GENERALLY USABLE CODE FOR DOUBLY LINKED LISTS	 *
 *****************************************************/

/*****************************************************
 *  Generic segment for every doubly linked list		 *
 *  A node of a specific list contains a member of	 *
 *  the type 'struct SoeListHdr' on top, e.g.:		 *
 *		struct MyNode {								 *
 *			struct SoeListHdr h;					 *
 *			...										 *
 *			<all other members you need>			 *
 *			...										 *
 *		}											 *
 *													 *
 *  The sentinel of the list will also be of type	 *
 *  'struct SoeListHdr'.								 *
 *****************************************************/

/* Dynamically allocate memory for a node			*/
void *SoeListNodeNew(size_t size) {
	return malloc(size);
}

/* Create a new list (= its sentinel)				*/
struct SoeListHdr *SoeListNew(void) {
	struct SoeListHdr *s;
	if ((s = (struct SoeListHdr *) SoeListNodeNew(sizeof ( struct SoeListHdr))) == NULL) return NULL;
	s->l = s;
	s->r = s;
	return (struct SoeListHdr *) s;
}

/* Add existing but yet unlinked node 'current'		 *
 *  before the (already linked) node 'next'			*/
void SoeListNodeAdd(struct SoeListHdr * current, struct SoeListHdr *next) {
	current->r = next;
	current->l = next->l;
	next->l->r = current;
	next->l = current;
}

/* Remove a node from the list and free its memory	*/
void * SoeListNodeRemove(struct SoeListHdr * p) {
	struct SoeListHdr *temp;
	if (p->r == p) return NULL;
	p->l->r = p->r;
	p->r->l = p->l;
	temp = p->r;
	free(p);
	return temp;
}

/* Remove a list (header) and free its memory	*/
int SoeListRemove(struct SoeListHdr * p) {
	struct SoeListHdr *n = p;
	int rc = 0;
	while (n->r != p) {
		n = n->r;
		free(n->l);
		rc++;
	}
	free(n);
	return rc;
}

/*****************************************************
 *  Print linkage information of node p to screen in	 *
 *  hexadecimal format:								 *
 *													 *
 *  previous hhhhhhhh								 *
 *  current  hhhhhhhh								 *
 *  next     hhhhhhhh								 *
 *****************************************************/
void SoeListHdrPrint(struct SoeListHdr * p) {
	printf("previous %p\ncurrent  %p\nnext     %p\n", p->l, p, p->r);
}
/*****************************************************
 *  END OF GENERALLY USABLE CODE FOR DOUBLY LINKED L. *
 *****************************************************/
