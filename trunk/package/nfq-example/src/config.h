// config.h

#define _U_

#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRId64 "lld"

#define INET6 1

#define HAVE_SOCKADDR_STORAGE 1
#undef HAVE_NET_PFVAR_H
#define HAVE_FCNTL_H 1

#define RETSIGTYPE void

#define USE_ETHER_NTOHOST 1
#define HAVE_NETINET_IF_ETHER_H 1
#define HAVE_STRUCT_ETHER_ADDR 1
