#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdint.h>

typedef uint16_t __be16;
typedef uint32_t __be32;
typedef uint16_t sa_family_t;
typedef uint32_t socklen_t;

struct in_addr {
	__be32	s_addr;
};

#define __SOCK_SIZE__	16		/* sizeof(struct sockaddr)	*/
struct sockaddr_in {
    sa_family_t	sin_family;	/* Address family		*/
    __be16		sin_port;	/* Port number			*/
    struct in_addr	sin_addr;	/* Internet address		*/

    /* Pad to size of `struct sockaddr'. */
    unsigned char   __pad[__SOCK_SIZE__ - sizeof(short int) -
			sizeof(unsigned short int) - sizeof(struct in_addr)];
};

struct sockaddr {
	sa_family_t	sa_family;	/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};

#define AF_INET		2	/* Internet IP Protocol 	*/
#define PF_INET		AF_INET

/**
 * enum sock_type - Socket types
 * @SOCK_STREAM: stream (connection) socket
 * @SOCK_DGRAM: datagram (conn.less) socket
 * @SOCK_RAW: raw socket
 * @SOCK_RDM: reliably-delivered message
 * @SOCK_SEQPACKET: sequential packet socket
 * @SOCK_DCCP: Datagram Congestion Control Protocol socket
 * @SOCK_PACKET: linux specific way of getting packets at the dev level.
 *		  For writing rarp and other similar things on the user level.
 *
 * When adding some new socket type please
 * grep ARCH_HAS_SOCKET_TYPE include/asm-* /socket.h, at least MIPS
 * overrides this enum for binary compat reasons.
 */
enum sock_type {
	SOCK_STREAM	= 1,
	SOCK_DGRAM	= 2,
	SOCK_RAW	= 3,
	SOCK_RDM	= 4,
	SOCK_SEQPACKET	= 5,
	SOCK_DCCP	= 6,
	SOCK_PACKET	= 10,
};

#define	ENOMEM		12	/* Out of memory */
#define	EINVAL		22	/* Invalid argument */

#define MSG_DONTWAIT	0x40	/* Nonblocking io		 */

#include <sys/types.h>
#include <stddef.h>

int socket(int domain, int type, int protocol);
int bind(int sockFD, const struct sockaddr *host_addr, int addrlen);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);

int close(int fd);

#endif // SOCKET_H_
