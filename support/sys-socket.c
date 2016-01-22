/*
 * <sys/socket.h> wrapper functions.
 *
 * Authors:
 *   Steffen Kiess (s-kiess@web.de)
 *
 * Copyright (C) 2015 Steffen Kiess
 */

#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>

#include <stddef.h>

#include "map.h"
#include "mph.h"

G_BEGIN_DECLS

int
Mono_Posix_SockaddrStorage_get_size (void)
{
	return sizeof (struct sockaddr_storage);
}

int
Mono_Posix_SockaddrUn_get_sizeof_sun_path (void)
{
	struct sockaddr_un sun;
	return sizeof (sun.sun_path);
}

int
Mono_Posix_FromInAddr (struct Mono_Posix_InAddr* source, void* destination)
{
	memcpy (&((struct in_addr*)destination)->s_addr, &source->s_addr, 4);
	return 0;
}

int
Mono_Posix_ToInAddr (void* source, struct Mono_Posix_InAddr* destination)
{
	memcpy (&destination->s_addr, &((struct in_addr*)source)->s_addr, 4);
	return 0;
}

int
Mono_Posix_FromIn6Addr (struct Mono_Posix_In6Addr* source, void* destination)
{
	memcpy (&((struct in6_addr*)destination)->s6_addr, &source->addr0, 16);
	return 0;
}

int
Mono_Posix_ToIn6Addr (void* source, struct Mono_Posix_In6Addr* destination)
{
	memcpy (&destination->addr0, &((struct in6_addr*)source)->s6_addr, 16);
	return 0;
}


int
Mono_Posix_Syscall_socketpair (int domain, int type, int protocol, int* socket1, int* socket2)
{
	int filedes[2] = {-1, -1};
	int r;

	r = socketpair (domain, type, protocol, filedes);

	*socket1 = filedes[0];
	*socket2 = filedes[1];
	return r;
}

int
Mono_Posix_Syscall_getsockopt (int socket, int level, int option_name, void* option_value, gint64* option_len)
{
	socklen_t len;
	int r;

	mph_return_if_socklen_t_overflow (*option_len);

	len = *option_len;

	r = getsockopt (socket, level, option_name, option_value, &len);

	*option_len = len;

	return r;
}

int
Mono_Posix_Syscall_getsockopt_timeval (int socket, int level, int option_name, struct Mono_Posix_Timeval* option_value)
{
	struct timeval tv;
	int r;
	socklen_t size;

	size = sizeof (struct timeval);
	r = getsockopt (socket, level, option_name, &tv, &size);

	if (r != -1 && size == sizeof (struct timeval)) {
		if (Mono_Posix_ToTimeval (&tv, option_value) != 0)
			return -1;
	} else {
		memset (option_value, 0, sizeof (struct Mono_Posix_Timeval));
		if (r != -1)
			errno = EINVAL;
	}

	return r;
}

int
Mono_Posix_Syscall_getsockopt_linger (int socket, int level, int option_name, struct Mono_Posix_Linger* option_value)
{
	struct linger ling;
	int r;
	socklen_t size;

	size = sizeof (struct linger);
	r = getsockopt (socket, level, option_name, &ling, &size);

	if (r != -1 && size == sizeof (struct linger)) {
		if (Mono_Posix_ToLinger (&ling, option_value) != 0)
			return -1;
	} else {
		memset (option_value, 0, sizeof (struct Mono_Posix_Linger));
		if (r != -1)
			errno = EINVAL;
	}

	return r;
}

int
Mono_Posix_Syscall_setsockopt (int socket, int level, int option_name, void* option_value, gint64 option_len)
{
	mph_return_if_socklen_t_overflow (option_len);

	return setsockopt (socket, level, option_name, option_value, option_len);
}

int
Mono_Posix_Syscall_setsockopt_timeval (int socket, int level, int option_name, struct Mono_Posix_Timeval* option_value)
{
	struct timeval tv;

	if (Mono_Posix_FromTimeval (option_value, &tv) != 0)
		return -1;

	return setsockopt (socket, level, option_name, &tv, sizeof (struct timeval));
}

int
Mono_Posix_Syscall_setsockopt_linger (int socket, int level, int option_name, struct Mono_Posix_Linger* option_value)
{
	struct linger ling;

	if (Mono_Posix_FromLinger (option_value, &ling) != 0)
		return -1;

	return setsockopt (socket, level, option_name, &ling, sizeof (struct linger));
}

static int
get_addrlen (struct Mono_Posix__SockaddrHeader* address, socklen_t* addrlen)
{
	if (!address) {
		*addrlen = 0;
		return 0;
	}

	switch (address->type) {
	case Mono_Posix_SockaddrType_SockaddrStorage:
		mph_return_if_socklen_t_overflow (((struct Mono_Posix__SockaddrDynamic*) address)->len);
		*addrlen = ((struct Mono_Posix__SockaddrDynamic*) address)->len;
		return 0;
	case Mono_Posix_SockaddrType_SockaddrUn:
		mph_return_if_socklen_t_overflow (offsetof (struct sockaddr_un, sun_path) + ((struct Mono_Posix__SockaddrDynamic*) address)->len);
		*addrlen = offsetof (struct sockaddr_un, sun_path) + ((struct Mono_Posix__SockaddrDynamic*) address)->len;
		return 0;
	case Mono_Posix_SockaddrType_Sockaddr: *addrlen = sizeof (struct sockaddr); return 0;
	case Mono_Posix_SockaddrType_SockaddrIn: *addrlen = sizeof (struct sockaddr_in); return 0;
	case Mono_Posix_SockaddrType_SockaddrIn6: *addrlen = sizeof (struct sockaddr_in6); return 0;
	default:
		*addrlen = 0;
		errno = EINVAL;
		return -1;
	}
}

int
Mono_Posix_Sockaddr_GetNativeSize (struct Mono_Posix__SockaddrHeader* address, gint64* size)
{
	socklen_t value;
	int r;

	r = get_addrlen (address, &value);
	*size = value;
	return r;
}

int
Mono_Posix_FromSockaddr (struct Mono_Posix__SockaddrHeader* source, void* destination)
{
	if (!source)
		return 0;

	switch (source->type) {
	case Mono_Posix_SockaddrType_SockaddrStorage:
		// Do nothing, don't copy source->sa_family into addr->sa_family
		return 0;

	case Mono_Posix_SockaddrType_SockaddrUn:
		memcpy (((struct sockaddr_un*) destination)->sun_path, ((struct Mono_Posix__SockaddrDynamic*) source)->data, ((struct Mono_Posix__SockaddrDynamic*) source)->len);
		break;

	case Mono_Posix_SockaddrType_Sockaddr:
		break;

	case Mono_Posix_SockaddrType_SockaddrIn:
		if (Mono_Posix_FromSockaddrIn ((struct Mono_Posix_SockaddrIn*) source, (struct sockaddr_in*) destination) != 0)
			return -1;
		break;

	case Mono_Posix_SockaddrType_SockaddrIn6:
		if (Mono_Posix_FromSockaddrIn6 ((struct Mono_Posix_SockaddrIn6*) source, (struct sockaddr_in6*) destination) != 0)
			return -1;
		break;

	default:
		errno = EINVAL;
		return -1;
	}

	int family;
	if (Mono_Posix_FromUnixAddressFamily (source->sa_family, &family) != 0)
		return -1;
	((struct sockaddr*) destination)->sa_family = family;

	return 0;
}

int
Mono_Posix_ToSockaddr (void* source, gint64 size, struct Mono_Posix__SockaddrHeader* destination)
{
	struct Mono_Posix__SockaddrDynamic* destination_dyn;

	if (!destination)
		return 0;

	switch (destination->type) {
	case Mono_Posix_SockaddrType_Sockaddr:
		if (size < offsetof (struct sockaddr, sa_family) + sizeof (sa_family_t)) {
			errno = ENOBUFS;
			return -1;
		}
		break;

	case Mono_Posix_SockaddrType_SockaddrStorage:
		destination_dyn = ((struct Mono_Posix__SockaddrDynamic*) destination);
		if (size > destination_dyn->len) {
			errno = ENOBUFS;
			return -1;
		}
		destination_dyn->len = size;
		break;

	case Mono_Posix_SockaddrType_SockaddrUn:
		destination_dyn = ((struct Mono_Posix__SockaddrDynamic*) destination);
		if (size - offsetof (struct sockaddr_un, sun_path) > destination_dyn->len) {
			errno = ENOBUFS;
			return -1;
		}
		destination_dyn->len = size - offsetof (struct sockaddr_un, sun_path);
		memcpy (destination_dyn->data, ((struct sockaddr_un*) source)->sun_path, size);
		break;

	case Mono_Posix_SockaddrType_SockaddrIn:
		if (size != sizeof (struct sockaddr_in)) {
			errno = ENOBUFS;
			return -1;
		}
		if (Mono_Posix_ToSockaddrIn ((struct sockaddr_in*) source, (struct Mono_Posix_SockaddrIn*) destination) != 0)
			return -1;
		break;

	case Mono_Posix_SockaddrType_SockaddrIn6:
		if (size != sizeof (struct sockaddr_in6)) {
			errno = ENOBUFS;
			return -1;
		}
		if (Mono_Posix_ToSockaddrIn6 ((struct sockaddr_in6*) source, (struct Mono_Posix_SockaddrIn6*) destination) != 0)
			return -1;
		break;

	default:
		errno = EINVAL;
		return -1;
	}

	if (Mono_Posix_ToUnixAddressFamily (((struct sockaddr*) source)->sa_family, &destination->sa_family) != 0)
		destination->sa_family = Mono_Posix_UnixAddressFamily_Unknown;

	return 0;
}

// Macro for allocating space for the native sockaddr_* structure
// Must be a macro because it is using alloca()

#define ALLOC_SOCKADDR                                                  \
    socklen_t addrlen;                                                  \
    struct sockaddr* addr;                                              \
    gboolean need_free = 0;                                             \
                                                                        \
    if (get_addrlen (address, &addrlen) != 0)                           \
        return -1;                                                      \
    if (address == NULL) {                                              \
        addr = NULL;                                                    \
    } else if (address->type == Mono_Posix_SockaddrType_SockaddrStorage) { \
        addr = (struct sockaddr*) ((struct Mono_Posix__SockaddrDynamic*) address)->data; \
    } else if (address->type == Mono_Posix_SockaddrType_SockaddrUn) { \
        /* Use alloca() for up to 2048 bytes, use malloc() otherwise */ \
        need_free = addrlen > 2048;                                     \
        addr = need_free ? malloc (addrlen) : alloca (addrlen);         \
        if (!addr)                                                      \
            return -1;                                                  \
    } else {                                                            \
        addr = alloca (addrlen);                                        \
    }


int
Mono_Posix_Syscall_bind (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	ALLOC_SOCKADDR
	if (Mono_Posix_FromSockaddr (address, addr) != 0) {
		if (need_free)
			free (addr);
		return -1;
	}

	r = bind (socket, addr, addrlen);

	if (need_free)
		free (addr);

	return r;
}

int
Mono_Posix_Syscall_connect (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	ALLOC_SOCKADDR
	if (Mono_Posix_FromSockaddr (address, addr) != 0) {
		if (need_free)
			free (addr);
		return -1;
	}

	r = connect (socket, addr, addrlen);

	if (need_free)
		free (addr);

	return r;
}

int
Mono_Posix_Syscall_accept (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	ALLOC_SOCKADDR

	r = accept (socket, addr, &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (addr, addrlen, address) != 0) {
		close (r);
		r = -1;
	}

	if (need_free)
		free (addr);

	return r;
}

int
Mono_Posix_Syscall_accept4 (int socket, struct Mono_Posix__SockaddrHeader* address, int flags)
{
#ifdef HAVE_ACCEPT4
	int r;

	ALLOC_SOCKADDR

	r = accept4 (socket, addr, &addrlen, flags);

	if (r != -1 && Mono_Posix_ToSockaddr (addr, addrlen, address) != 0) {
		close (r);
		r = -1;
	}

	if (need_free)
		free (addr);

	return r;
#else
	errno = EINVAL;
	return -1;
#endif
}

int
Mono_Posix_Syscall_getpeername (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	ALLOC_SOCKADDR

	r = getpeername (socket, addr, &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (addr, addrlen, address) != 0)
		r = -1;

	if (need_free)
		free (addr);

	return r;
}

int
Mono_Posix_Syscall_getsockname (int socket, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	ALLOC_SOCKADDR

	r = getsockname (socket, addr, &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (addr, addrlen, address) != 0)
		r = -1;

	if (need_free)
		free (addr);

	return r;
}

gint64
Mono_Posix_Syscall_recv (int socket, void* message, guint64 length, int flags)
{
	mph_return_if_size_t_overflow (length);

	return recv (socket, message, length, flags);
}

gint64
Mono_Posix_Syscall_send (int socket, void* message, guint64 length, int flags)
{
	mph_return_if_size_t_overflow (length);

	return send (socket, message, length, flags);
}

gint64
Mono_Posix_Syscall_recvfrom (int socket, void* buffer, guint64 length, int flags, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	mph_return_if_size_t_overflow (length);

	ALLOC_SOCKADDR

	r = recvfrom (socket, buffer, length, flags, addr, &addrlen);

	if (r != -1 && Mono_Posix_ToSockaddr (addr, addrlen, address) != 0)
		r = -1;

	if (need_free)
		free (addr);

	return r;
}

gint64
Mono_Posix_Syscall_sendto (int socket, void* message, guint64 length, int flags, struct Mono_Posix__SockaddrHeader* address)
{
	int r;

	mph_return_if_size_t_overflow (length);

	ALLOC_SOCKADDR
	if (Mono_Posix_FromSockaddr (address, addr) != 0) {
		if (need_free)
			free (addr);
		return -1;
	}

	r = sendto (socket, message, length, flags, addr, addrlen);

	if (need_free)
		free (addr);

	return r;
}

// vim: noexpandtab
// Local Variables: 
// tab-width: 4
// c-basic-offset: 4
// indent-tabs-mode: t
// End: 