/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include <linux/err.h>
#include <asm/mach-types.h>
#include <mach/msm_rpcrouter.h>

#define RAPP_RPC_PROG				0x30000010
#define RAPP_RPC_VER				0x0

#define RAPP_RPC_TIMEOUT (5*HZ)

#define RAPP_ENABLE_DS_CHANNEL_PROC		3


static struct msm_rpc_endpoint *rapp_ep,*call_ep;
int msm_rapp_rpc_connect(void)
{

	if (rapp_ep && !IS_ERR(rapp_ep)) {
		pr_info("%s: usb_ep already connected\n", __func__);
		return 0;
	}
	rapp_ep = msm_rpc_connect(RAPP_RPC_PROG, RAPP_RPC_VER, MSM_RPC_UNINTERRUPTIBLE);

	if (IS_ERR(rapp_ep)) {
		pr_err("%s: connect failed vers = %x\n",
				__func__, RAPP_RPC_VER);
		return -EAGAIN;
	} else
		pr_debug("%s: rpc connect success vers = %x\n",
				__func__, RAPP_RPC_VER);
	
/*	call_ep = msm_rpc_connect(0x30000000, 0xdfe0120c, MSM_RPC_UNINTERRUPTIBLE);
	if (IS_ERR(call_ep)) {
                pr_err("%s: connect failed vers = %x\n",
                                __func__, 0xdfe0120c);
        } else
                pr_err("%s: rpc connect success vers = %x\n",
                                __func__, 0xdfe0120c);*/
	return 0;
}
EXPORT_SYMBOL(msm_rapp_rpc_connect);


int msm_rapc_enable_ds_channel(void)
{
	int rc = 0;
	struct hsusb_phy_start_req {
		struct rpc_request_hdr hdr;
		uint32_t data1;
		uint32_t data2;
		uint32_t data3;
	} req;

	if (!rapp_ep || IS_ERR(rapp_ep)) {
		pr_err("%s: rpc connect failed: rc = %ld\n",
			__func__, PTR_ERR(rapp_ep));
		return -EAGAIN;
	}

	req.data1 = cpu_to_be32(0x1);
	req.data2 = cpu_to_be32(0xd);
	req.data3 = cpu_to_be32(0xffffffff);
	rc = msm_rpc_call(rapp_ep, RAPP_ENABLE_DS_CHANNEL_PROC,
				&req, sizeof(req),
				5 * HZ);
	if (rc < 0)
		pr_err("%s: rpc call failed! error: %d\n",
			__func__, rc);
	else
		pr_debug("%s: rpc call success\n" , __func__);

	return rc;
}
EXPORT_SYMBOL(msm_rapc_enable_ds_channel);
