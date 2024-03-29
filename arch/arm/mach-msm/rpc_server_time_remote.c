/* arch/arm/mach-msm/rpc_server_time_remote.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Author: Iliyan Malchev <ibm@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>
#include <mach/msm_rpc_version.h>
#include <mach/amss_para.h>

/* time_remote_mtoa server definitions. */

#define TIME_REMOTE_MTOA_PROG 0x3000005d
#define RPC_TIME_REMOTE_MTOA_NULL   0
#define RPC_TIME_TOD_SET_APPS_BASES 2

struct rpc_time_tod_set_apps_bases_args {
	uint32_t tick;
	uint64_t stamp;
};

static int handle_rpc_call(struct msm_rpc_server *server,
			   struct rpc_request_hdr *req, unsigned len)
{
	switch (req->procedure) {
	case RPC_TIME_REMOTE_MTOA_NULL:
		return 0;

	case RPC_TIME_TOD_SET_APPS_BASES: {
		struct rpc_time_tod_set_apps_bases_args *args;
		args = (struct rpc_time_tod_set_apps_bases_args *)(req + 1);
		args->tick = be32_to_cpu(args->tick);
		args->stamp = be64_to_cpu(args->stamp);
		printk(KERN_INFO "RPC_TIME_TOD_SET_APPS_BASES:\n"
		       "\ttick = %d\n"
		       "\tstamp = %lld\n",
		       args->tick, args->stamp);
		return 0;
	}
	default:
		printk ("Ajeet: do something\n");
		return -ENODEV;
	}
}

static struct msm_rpc_server rpc_server = {
	.prog = TIME_REMOTE_MTOA_PROG,
	.rpc_call = handle_rpc_call,
};

static int __init rpc_server_init(void)
{
	/* Dual server registration to support backwards compatibility vers */
	rpc_server.vers = amss_get_num_value(TIME_REMOTE_MTOA_VERS);
	return msm_rpc_create_server(&rpc_server);
}


module_init(rpc_server_init);
