/*
 * Copyright 2010-2013 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, version 2.1.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * Disclaimer: The codes contained in these modules may be specific
 * to the Intel Software Development Platform codenamed Knights Ferry,
 * and the Intel product codenamed Knights Corner, and are not backward
 * compatible with other Intel products. Additionally, Intel will NOT
 * support the codes or instruction set in future products.
 *
 * Intel offers no warranty of any kind regarding the code. This code is
 * licensed on an "AS IS" basis and Intel is not obligated to provide
 * any support, assistance, installation, training, or other services
 * of any kind. Intel is also not obligated to provide any updates,
 * enhancements or extensions. Intel specifically disclaims any warranty
 * of merchantability, non-infringement, fitness for any particular
 * purpose, and any other warranty.
 *
 * Further, Intel disclaims all liability of any kind, including but
 * not limited to liability for infringement of any proprietary rights,
 * relating to the use of the code, even if Intel is notified of the
 * possibility of such liability. Except as expressly stated in an Intel
 * license agreement provided with this code and agreed upon with Intel,
 * no license, express or implied, by estoppel or otherwise, to any
 * intellectual property rights is granted herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "stdio.h"
#include <linux/errno.h>
#include <stdint.h>
#include <errno.h>

#include "scif.h"
#include "scif_ioctl.h"


#define DEVICE_NODE "/dev/mic/scif"

#define __symver_tag SCIF

#define __symver(S, at, M, N) \
	__asm__ (".symver " __str(V(S, M, N)) "," __V(S, at, M, N));
#define __V(symbol, at, major, minor) \
	__str(symbol) at __str(__symver_tag) "_" __str(major) "." __str(minor)
#define __str(s) ___str(s)
#define ___str(s) #s

#ifndef GENMAP_PARSING_PASS
#define V(symbol, major, minor) \
	__ ## symbol ## _ ## major ## _ ## minor
#define compatible_version(symbol, major, minor) \
	__symver(symbol, "@", major, minor)
#define default_version(symbol, major, minor) \
	__symver(symbol, "@@", major, minor)
#define only_version(symbol, major, minor)
#endif


static uint8_t scif_version_mismatch;

static int
scif_get_driver_version(void)
{
	int scif_version;
	scif_epd_t fd;

	if ((fd = open(DEVICE_NODE, O_RDWR)) < 0)
		return -1;
	scif_version = ioctl(fd, SCIF_GET_VERSION);
	if (scif_version < 0) {
		close(fd);
		return -1;
	}
	close(fd);
	return scif_version;
}

scif_epd_t
scif_open(void)
{
	scif_epd_t fd;

	if (scif_version_mismatch) {
		errno = ENXIO;
		return -1;
	}

	if ((fd = open(DEVICE_NODE, O_RDWR)) < 0)
		return (scif_epd_t)-1;

	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
		close(fd);
		return (scif_epd_t)-1;
	}
	return fd;
}
only_version(scif_open, 0, 0)

int
scif_close(scif_epd_t epd)
{
	if (close(epd))
		return -1;
	return 0;
}
only_version(scif_close, 0, 0)

int
scif_bind(scif_epd_t epd, uint16_t pn)
{
	int pni = pn;

	if (ioctl(epd, SCIF_BIND, &pni) < 0)
		return -1;

	return pni;
}
only_version(scif_bind, 0, 0)

int
scif_listen(scif_epd_t epd, int backlog)
{
	if (ioctl(epd, SCIF_LISTEN, backlog) < 0)
		return -1;

	return 0;
}
only_version(scif_listen, 0, 0)

int
scif_connect(scif_epd_t epd, struct scif_portID *dst)
{
	struct scifioctl_connect req;

	if (!dst) {
		errno = EINVAL;
		return -1;
	}

	req.peer.node = dst->node;
	req.peer.port = dst->port;

	if (ioctl(epd, SCIF_CONNECT, &req) < 0)
		return -1;

	return req.self.port;
}
only_version(scif_connect, 0, 0)

int
scif_accept(scif_epd_t epd, struct scif_portID *peer, scif_epd_t *newepd, int flags)
{
	struct scifioctl_accept req;
	scif_epd_t newfd;
	int errno_save;

	if (!peer || !newepd) {
		errno = EINVAL;
		return -1;
	}

	// Create a new file descriptor link it to the new connection
	if ((newfd = open(DEVICE_NODE, O_RDWR)) < 0)
		return -1;

	req.flags = flags;

	// First get the accept connection completed
	if (ioctl(epd, SCIF_ACCEPTREQ, &req) < 0) {
		errno_save = errno;
		close(newfd);
		errno = errno_save;
		return -1;
	}

	if (ioctl(newfd, SCIF_ACCEPTREG, &req.endpt) < 0) {
		errno_save = errno;
		close(newfd);
		errno = errno_save;
		return -1;
	}

	*newepd = newfd;
	peer->node = req.peer.node;
	peer->port = req.peer.port;
	return 0;
}
only_version(scif_accept, 0, 0)

int
scif_send(scif_epd_t epd, void *msg, int len, int flags)
{
	struct scifioctl_msg send_msg =
		{ .msg = msg, .len = len, .flags = flags };
	if (ioctl(epd, SCIF_SEND, &send_msg) < 0)
		return -1;
	return send_msg.out_len;
}
only_version(scif_send, 0, 0)

int
scif_recv(scif_epd_t epd, void *msg, int len, int flags)
{
	struct scifioctl_msg recv_msg =
		{ .msg = msg, .len = len, .flags = flags };

	if (ioctl(epd, SCIF_RECV, &recv_msg) < 0)
		return -1;
	return recv_msg.out_len;
}
only_version(scif_recv, 0, 0)

off_t
scif_register(scif_epd_t epd, void *addr, size_t len, off_t offset,
				int prot, int flags)
{
	struct scifioctl_reg reg =
	{ .addr = addr, .len = len, .offset = offset,
			.prot = prot, .flags = flags };

	if (ioctl(epd, SCIF_REG, &reg) < 0)
		return -1;
	return reg.out_offset;
}
only_version(scif_register, 0, 0)

int
scif_unregister(scif_epd_t epd, off_t offset, size_t len)
{
	struct scifioctl_unreg unreg =
		{ .len = len, .offset = offset};

	if (ioctl(epd, SCIF_UNREG, &unreg) < 0)
		return -1;
	return 0;
}
only_version(scif_unregister, 0, 0)

void*
scif_mmap(void *addr, size_t len, int prot, int flags, scif_epd_t epd, off_t offset)
{
	return mmap(addr, len, prot, (flags | MAP_SHARED), (int)epd, offset);
}
only_version(scif_mmap, 0, 0)

int
scif_munmap(void *addr, size_t len)
{
	return munmap(addr, len);
}
only_version(scif_munmap, 0, 0)

int
scif_readfrom(scif_epd_t epd, off_t loffset, size_t len, off_t roffset, int flags)
{
	struct scifioctl_copy copy =
	{.loffset = loffset, .len = len, .roffset = roffset, .flags = flags };

	if (ioctl(epd, SCIF_READFROM, &copy) < 0)
		return -1;
	return 0;
}
only_version(scif_readfrom, 0, 0)

int
scif_writeto(scif_epd_t epd, off_t loffset, size_t len, off_t roffset, int flags)
{
	struct scifioctl_copy copy =
	{.loffset = loffset, .len = len, .roffset = roffset, .flags = flags };

	if (ioctl(epd, SCIF_WRITETO, &copy) < 0)
		return -1;
	return 0;
}
only_version(scif_writeto, 0, 0)

int
scif_vreadfrom(scif_epd_t epd, void *addr, size_t len, off_t offset, int flags)
{
	struct scifioctl_copy copy =
	{.addr = addr, .len = len, .roffset = offset, .flags = flags };

	if (ioctl(epd, SCIF_VREADFROM, &copy) < 0)
		return -1;
	return 0;
}
only_version(scif_vreadfrom, 0, 0)

int
scif_vwriteto(scif_epd_t epd, void *addr, size_t len, off_t offset, int flags)
{
	struct scifioctl_copy copy =
	{.addr = addr, .len = len, .roffset = offset, .flags = flags };

	if (ioctl(epd, SCIF_VWRITETO, &copy) < 0)
		return -1;
	return 0;
}
only_version(scif_vwriteto, 0, 0)

int
scif_fence_mark(scif_epd_t epd, int flags, int *mark)
{
	struct scifioctl_fence_mark fence_mark =
	{.flags = flags, .mark = mark};

	if (ioctl(epd, SCIF_FENCE_MARK, &fence_mark) < 0)
		return -1;
	return 0;
}
only_version(scif_fence_mark, 0, 0)

int
scif_fence_wait(scif_epd_t epd, int mark)
{
	if (ioctl(epd, SCIF_FENCE_WAIT, mark) < 0)
		return -1;
	return 0;
}
only_version(scif_fence_wait, 0, 0)

int
scif_fence_signal(scif_epd_t epd, off_t loff, uint64_t lval,
	off_t roff, uint64_t rval, int flags)
{
	struct scifioctl_fence_signal signal =
	{.loff = loff, .lval = lval, .roff = roff,
		.rval = rval, .flags = flags};

	if (ioctl(epd, SCIF_FENCE_SIGNAL, &signal) < 0)
		return -1;

	return 0;
}
only_version(scif_fence_signal, 0, 0)

int
scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self)
{
	scif_epd_t fd;
	struct scifioctl_nodeIDs nodeIDs = {.nodes = nodes, .len = len, .self = self};

	if ((fd = open(DEVICE_NODE, O_RDWR)) < 0)
		return -1;

	if (ioctl(fd, SCIF_GET_NODEIDS, &nodeIDs) < 0) {
		close(fd);
		return -1;
	}

	close(fd);

	return nodeIDs.len;
}
only_version(scif_get_nodeIDs, 0, 0)

int
scif_poll(struct scif_pollepd *ufds, unsigned int nfds, long timeout_msecs)
{
	return poll((struct pollfd*)ufds, nfds, timeout_msecs);
}
only_version(scif_poll, 0, 0)

__attribute__ ((constructor)) static void scif_lib_init(void)
{
	int scif_driver_ver = scif_get_driver_version();
	if ((scif_driver_ver > 0) && (scif_driver_ver != SCIF_VERSION))
		scif_version_mismatch = 1;
}
