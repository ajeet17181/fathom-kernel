/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __DAL_AUDIO_H__
#define __DAL_AUDIO_H__

#include "dal_audio_format.h"
#include "dal.h"
#include <mach/msm_qdsp6_audio.h>

#define AUDIO_DAL_DEVICE 0x02000028
#define AUDIO_DAL_PORT "DSP_DAL_AQ_AUD"

enum 
{
#if defined(CONFIG_MACH_HTCLEO)
	AUDIO_OP_OPEN = DAL_OP_FIRST_DEVICE_API,
	AUDIO_OP_WRITE,
	AUDIO_OP_READ,
	AUDIO_OP_IOCTL,
	AUDIO_OP_INIT,
	AUDIO_OP_CLOSE,
	AUDIO_OP_FLUSH
#else
	AUDIO_OP_CONTROL = DAL_OP_FIRST_DEVICE_API,
	AUDIO_OP_DATA,
	AUDIO_OP_INIT,

#endif
};  

/* ---- common audio structures ---- */

/* This flag, if set, indicates that the beginning of the data in the*/
/* buffer is a synchronization point or key frame, meaning no data */
/* before it in the stream is required in order to render the stream */
/* from this point onward. */
#define ADSP_AUDIO_BUFFER_FLAG_SYNC_POINT        0x01

/* This flag, if set, indicates that the buffer object is using valid */
/* physical address used to store the media data */
#define ADSP_AUDIO_BUFFER_FLAG_PHYS_ADDR         0x04

/* This flag, if set, indicates that a media start timestamp has been */
/* set for a buffer. */
#define ADSP_AUDIO_BUFFER_FLAG_START_SET         0x08

/* This flag, if set, indicates that a media stop timestamp has been set */
/* for a buffer. */
#define ADSP_AUDIO_BUFFER_FLAG_STOP_SET          0x10

/* This flag, if set, indicates that a preroll timestamp has been set */
/* for a buffer. */
#define ADSP_AUDIO_BUFFER_FLAG_PREROLL_SET       0x20

/* This flag, if set, indicates that the data in the buffer is a fragment of */
/* a larger block of data, and will be continued by the data in the next */
/* buffer to be delivered. */
#define ADSP_AUDIO_BUFFER_FLAG_CONTINUATION      0x40

struct adsp_audio_buffer {
	u32 addr;       /* Physical Address of buffer */
	u32 max_size;       /* Maximum size of buffer */
	u32 actual_size;    /* Actual size of valid data in the buffer */
	u32 offset;     /* Offset to the first valid byte */
	u32 flags;      /* ADSP_AUDIO_BUFFER_FLAGs that has been set */
	s64 start;      /* Start timestamp, if any */
	s64 stop;       /* Stop timestamp, if any */
	s64 preroll;        /* Preroll timestamp, if any */
} __attribute__ ((packed));



/* ---- audio commands ---- */

/* Command/event response types */
#define ADSP_AUDIO_RESPONSE_COMMAND   0
#define ADSP_AUDIO_RESPONSE_ASYNC     1

#if !defined(CONFIG_MACH_HTCLEO)
struct adsp_command_hdr {
	u32 size;       /* sizeof(cmd) - sizeof(u32) */

	u32 dst;
	u32 src;

	u32 opcode;
	u32 response_type;
	u32 seq_number;

	u32 context;        /* opaque to DSP */
	u32 data;

	u32 padding;
} __attribute__ ((packed));

#else

struct adsp_command_hdr 
{
	u32 context; 
	u32 data;    
} __attribute__ ((packed));
#endif

#define AUDIO_DOMAIN_APP    0
#define AUDIO_DOMAIN_MODEM  1
#define AUDIO_DOMAIN_DSP    2

#define AUDIO_SERVICE_AUDIO 0
#define AUDIO_SERVICE_VIDEO 1 /* really? */

/* adsp audio addresses are (byte order) domain, service, major, minor */
//#define AUDIO_ADDR(maj,min) ( (((maj) & 0xff) << 16) | (((min) & 0xff) << 24) | (1) )

#define AUDIO_ADDR(maj,min,dom) ( (((min) & 0xff) << 24) | (((maj) & 0xff) << 16) | ((AUDIO_SERVICE_AUDIO) << 8) | (dom) )


/* AAC Encoder modes */
#define ADSP_AUDIO_ENC_AAC_LC_ONLY_MODE     0
#define ADSP_AUDIO_ENC_AAC_PLUS_MODE        1
#define ADSP_AUDIO_ENC_ENHANCED_AAC_PLUS_MODE   2

struct adsp_audio_aac_enc_cfg {
	u32 bit_rate;       /* bits per second */
	u32 encoder_mode;   /* ADSP_AUDIO_ENC_* */
} __attribute__ ((packed));

#define ADSP_AUDIO_ENC_SBC_ALLOCATION_METHOD_LOUNDNESS     0
#define ADSP_AUDIO_ENC_SBC_ALLOCATION_METHOD_SNR           1

#define ADSP_AUDIO_ENC_SBC_CHANNEL_MODE_MONO                1
#define ADSP_AUDIO_ENC_SBC_CHANNEL_MODE_STEREO              2
#define ADSP_AUDIO_ENC_SBC_CHANNEL_MODE_DUAL                8
#define ADSP_AUDIO_ENC_SBC_CHANNEL_MODE_JOINT_STEREO        9

struct adsp_audio_sbc_encoder_cfg {
	u32 num_subbands;
	u32 block_len;
	u32 channel_mode;
	u32 allocation_method;
	u32 bit_rate;
} __attribute__ ((packed));

/* AMR NB encoder modes */
#define ADSP_AUDIO_AMR_MR475    0
#define ADSP_AUDIO_AMR_MR515    1
#define ADSP_AUDIO_AMR_MMR59    2
#define ADSP_AUDIO_AMR_MMR67    3
#define ADSP_AUDIO_AMR_MMR74    4
#define ADSP_AUDIO_AMR_MMR795   5
#define ADSP_AUDIO_AMR_MMR102   6
#define ADSP_AUDIO_AMR_MMR122   7

/* The following are valid AMR NB DTX modes */
#define ADSP_AUDIO_AMR_DTX_MODE_OFF     0
#define ADSP_AUDIO_AMR_DTX_MODE_ON_VAD1     1
#define ADSP_AUDIO_AMR_DTX_MODE_ON_VAD2     2
#define ADSP_AUDIO_AMR_DTX_MODE_ON_AUTO     3

/* AMR Encoder configuration */
struct adsp_audio_amr_enc_cfg {
	u32 mode;       /* ADSP_AUDIO_AMR_MR* */
	u32 dtx_mode;   /* ADSP_AUDIO_AMR_DTX_MODE* */
	u32 enable;     /* 1 = enable, 0 = disable */
} __attribute__ ((packed));

struct adsp_audio_qcelp13k_enc_cfg {
	u16 min_rate;
	u16 max_rate;
} __attribute__ ((packed));

struct adsp_audio_evrc_enc_cfg {
	u16 min_rate;
	u16 max_rate;
} __attribute__ ((packed));

union adsp_audio_codec_config {
	struct adsp_audio_amr_enc_cfg amr;
	struct adsp_audio_aac_enc_cfg aac;
	struct adsp_audio_qcelp13k_enc_cfg qcelp13k;
	struct adsp_audio_evrc_enc_cfg evrc;
	struct adsp_audio_sbc_encoder_cfg sbc;
} __attribute__ ((packed));


/* This is the default value. */
#define ADSP_AUDIO_OPEN_STREAM_MODE_NONE        0x0000

/* This bit, if set, indicates that the AVSync mode is activated. */
#define ADSP_AUDIO_OPEN_STREAM_MODE_AVSYNC      0x0001

/* This bit, if set, indicates that the Sample Rate/Channel Mode */
/* Change Notification mode is activated. */
#define ADSP_AUDIO_OPEN_STREAM_MODE_SR_CM_NOTIFY    0x0002

/* This bit, if set, indicates that the sync clock is enabled */
#define  ADSP_AUDIO_OPEN_STREAM_MODE_ENABLE_SYNC_CLOCK  0x0004

#if !defined(CONFIG_MACH_HTCLEO)
struct adsp_open_command {
	struct adsp_command_hdr hdr;

	u32 device;
	u32 endpoint; /* address */

	u32 stream_context;
	u32 mode;

	u32 buf_max_size;

	union adsp_audio_format format;
	union adsp_audio_codec_config config;
} __attribute__ ((packed));

/* Opcode to open a device stream session to capture audio */
#define ADSP_AUDIO_IOCTL_CMD_OPEN_READ          0x0108dd79

/* Opcode to open a device stream session to render audio */
#define ADSP_AUDIO_IOCTL_CMD_OPEN_WRITE         0x0108dd7a

/* Opcode to open a device session, must open a device */
#define ADSP_AUDIO_IOCTL_CMD_OPEN_DEVICE        0x0108dd7b

/* Close an existing stream or device */
#define ADSP_AUDIO_IOCTL_CMD_CLOSE          0x0108d8bc

#else

struct adsp_open_command 
{
	struct adsp_command_hdr hdr;
	    
	u32 opcode;
	u32 numdev;
	u32 dev[4];
	u32 stream_context;

	u32 format;    
	u32 pblock;
	u32 blocklen;

	u32 buf_max_size;
	u32 priority;

	union adsp_audio_codec_config config;
	u32 mode;
} __attribute__ ((packed));

/* --- audio control and stream session ioctls ---- */

/* Opcode to open a device stream session to capture audio */
#define ADSP_AUDIO_OPCODE_OPEN_READ          0x01

/* Opcode to open a device stream session to render audio */
#define ADSP_AUDIO_OPCODE_OPEN_WRITE          0x02

/* Opcode to open a device session, must open a device */
#define ADSP_AUDIO_OPCODE_OPEN_DEV          0x04

#endif

/* A device switch requires three IOCTL */
/* commands in the following sequence: PREPARE, STANDBY, COMMIT */

/* adsp_audio_device_switch_command structure is needed for */
/* DEVICE_SWITCH_PREPARE */

/* Device switch protocol step #1. Pause old device and */
/* generate silence for the old device. */
#define ADSP_AUDIO_IOCTL_CMD_DEVICE_SWITCH_PREPARE  0x010815c4

/* Device switch protocol step #2. Release old device, */
/* create new device and generate silence for the new device. */

/* When client receives ack for this IOCTL, the client can */
/* start sending IOCTL commands to configure, calibrate and */
/* change filter settings on the new device. */
#define ADSP_AUDIO_IOCTL_CMD_DEVICE_SWITCH_STANDBY  0x010815c5

/* Device switch protocol step #3. Start normal operations on new device */
#define ADSP_AUDIO_IOCTL_CMD_DEVICE_SWITCH_COMMIT   0x01075ee7

struct adsp_device_switch_command {
	struct adsp_command_hdr hdr;
	u32 old_device;
	u32 new_device;
	u8 device_class; /* 0 = i.rx, 1 = i.tx, 2 = e.rx, 3 = e.tx */
	u8 device_type; /* 0 = rx, 1 = tx, 2 = both */
} __attribute__ ((packed));



/* --- audio control session ioctls ---- */

#define ADSP_PATH_RX    0
#define ADSP_PATH_TX    1
#define ADSP_PATH_BOTH  2

/* These commands will affect a logical device and all its associated */
/* streams. */


/* Set device volume. */
#define ADSP_AUDIO_IOCTL_CMD_SET_DEVICE_VOL     0x0107605c

struct adsp_set_dev_volume_command {
	struct adsp_command_hdr hdr;
	u32 device_id;
	u32 path; /* 0 = rx, 1 = tx, 2 = both */
	s32 volume;
} __attribute__ ((packed));

/* Set Device stereo volume. This command has data payload, */
/* struct adsp_audio_set_dev_stereo_volume_command. */
#define ADSP_AUDIO_IOCTL_SET_DEVICE_STEREO_VOL      0x0108df3e

/* Set L, R cross channel gain for a Device. This command has */
/* data payload, struct adsp_audio_set_dev_x_chan_gain_command. */
#define ADSP_AUDIO_IOCTL_SET_DEVICE_XCHAN_GAIN      0x0108df40

/* Set device mute state. */
#define ADSP_AUDIO_IOCTL_CMD_SET_DEVICE_MUTE        0x0107605f

struct adsp_set_dev_mute_command {
	struct adsp_command_hdr hdr;
	u32 device_id;
	u32 path; /* 0 = rx, 1 = tx, 2 = both */
	u32 mute; /* 1 = mute */
} __attribute__ ((packed));

/* Configure Equalizer for a device. */
/* This command has payload struct adsp_audio_set_dev_equalizer_command. */
#define ADSP_AUDIO_IOCTL_CMD_SET_DEVICE_EQ_CONFIG   0x0108b10e

/* Set configuration data for an algorithm aspect of a device. */
/* This command has payload struct adsp_audio_set_dev_cfg_command. */
#define ADSP_AUDIO_IOCTL_SET_DEVICE_CONFIG      0x0108b6cb

struct adsp_set_dev_cfg_command {
	struct adsp_command_hdr hdr;
	u32 device_id;
	u32 block_id;
	u32 interface_id;
	u32 phys_addr;
	u32 phys_size;
	u32 phys_used;
} __attribute__ ((packed));

/* Set configuration data for all interfaces of a device. */
#define ADSP_AUDIO_IOCTL_SET_DEVICE_CONFIG_TABLE    0x0108b6bf

struct adsp_set_dev_cfg_table_command {
	struct adsp_command_hdr hdr;
	u32 device_id;
	u32 phys_addr;
	u32 phys_size;
	u32 phys_used;
} __attribute__ ((packed));

/* ---- audio stream data commands ---- */

#define ADSP_AUDIO_IOCTL_CMD_DATA_TX            0x0108dd7f
#define ADSP_AUDIO_IOCTL_CMD_DATA_RX            0x0108dd80

struct adsp_buffer_command {
	struct adsp_command_hdr hdr;
	struct adsp_audio_buffer buffer;
} __attribute__ ((packed));



/* ---- audio stream ioctls (only affect a single stream in a session) ---- */

/* Stop stream for audio device. */
#define ADSP_AUDIO_IOCTL_CMD_STREAM_STOP        0x01075c54

/* End of stream reached. Client will not send any more data. */
#define ADSP_AUDIO_IOCTL_CMD_STREAM_EOS         0x0108b150

/* Do sample slipping/stuffing on AAC outputs. The payload of */
/* this command is struct adsp_audio_slip_sample_command. */
#define ADSP_AUDIO_IOCTL_CMD_STREAM_SLIPSAMPLE      0x0108d40e

/* Set stream volume. */
/* This command has data payload, struct adsp_audio_set_volume_command. */
#define ADSP_AUDIO_IOCTL_CMD_SET_STREAM_VOL     0x0108c0de

/* Set stream stereo volume. This command has data payload, */
/* struct adsp_audio_set_stereo_volume_command. */
#define ADSP_AUDIO_IOCTL_SET_STREAM_STEREO_VOL      0x0108dd7c

/* Set L, R cross channel gain for a Stream. This command has */
/* data payload, struct adsp_audio_set_x_chan_gain_command. */
#define ADSP_AUDIO_IOCTL_SET_STREAM_XCHAN_GAIN      0x0108dd7d

/* Set stream mute state. */
/* This command has data payload, struct adsp_audio_set_stream_mute. */
#define ADSP_AUDIO_IOCTL_CMD_SET_STREAM_MUTE        0x0108c0df

/* Reconfigure bit rate information. This command has data */
/* payload, struct adsp_audio_set_bit_rate_command */
#define ADSP_AUDIO_IOCTL_SET_STREAM_BITRATE     0x0108ccf1

/* Set Channel Mapping. This command has data payload, struct */
/* This command has data payload struct adsp_audio_set_channel_map_command. */
#define ADSP_AUDIO_IOCTL_SET_STREAM_CHANNELMAP      0x0108d32a

/* Enable/disable AACPlus SBR. */
/* This command has data payload struct adsp_audio_set_sbr_command */
#define ADSP_AUDIO_IOCTL_SET_STREAM_SBR         0x0108d416

/* Enable/disable WMA Pro Chex and Fex. This command has data payload */
/* struct adsp_audio_stream_set_wma_command. */
#define ADSP_AUDIO_IOCTL_SET_STREAM_WMAPRO      0x0108d417


/* ---- audio session ioctls (affect all streams in a session) --- */

/* Start stream for audio device. */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_START      0x010815c6

/* Stop all stream(s) for audio session as indicated by major id. */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_STOP       0x0108dd7e

/* Pause the data flow for a session as indicated by major id. */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_PAUSE      0x01075ee8

/* Resume the data flow for a session as indicated by major id. */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_RESUME     0x01075ee9

/* Drop any unprocessed data buffers for a session as indicated by major id. */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_FLUSH      0x01075eea

/* Start Stream DTMF tone */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_DTMF_START     0x0108c0dd

/* Stop Stream DTMF tone */
#define ADSP_AUDIO_IOCTL_CMD_SESSION_DTMF_STOP      0x01087554

/* Set Session volume. */
/* This command has data payload, struct adsp_audio_set_volume_command. */
#define ADSP_AUDIO_IOCTL_SET_SESSION_VOL        0x0108d8bd

/* Set session stereo volume. This command has data payload, */
/* struct adsp_audio_set_stereo_volume_command. */
#define ADSP_AUDIO_IOCTL_SET_SESSION_STEREO_VOL     0x0108df3d

/* Set L, R cross channel gain for a session. This command has */
/* data payload, struct adsp_audio_set_x_chan_gain_command. */
#define ADSP_AUDIO_IOCTL_SET_SESSION_XCHAN_GAIN     0x0108df3f

/* Set Session mute state. */
/* This command has data payload, struct adsp_audio_set_mute_command. */
#define ADSP_AUDIO_IOCTL_SET_SESSION_MUTE       0x0108d8be

/* Configure Equalizer for a stream. */
/* This command has payload struct adsp_audio_set_equalizer_command. */
#define ADSP_AUDIO_IOCTL_SET_SESSION_EQ_CONFIG      0x0108c0e0

#define ADSP_AUDIO_MAX_EQ_BANDS     12

/* Definition for any one band of Equalizer. */
struct adsp_audio_eq_band {
	u16 band_idx;
	u32 filter_type;
	u32 center_freq_hz;
	s32 filter_gain;
	s32 q_factor;
} __attribute__ ((packed));

struct adsp_audio_set_equalizer_command {
	struct adsp_command_hdr hdr;
	u32 enable;
	u32 num_bands;
	struct adsp_audio_eq_band eq_bands[ADSP_AUDIO_MAX_EQ_BANDS];
} __attribute__ ((packed));


/* Set Audio Video sync information. */
/* This command has data payload, struct adsp_audio_set_av_sync_command. */
#define ADSP_AUDIO_IOCTL_SET_SESSION_AVSYNC     0x0108d1e2

/* Get Audio Media Session time. */
/* This command returns the audioTime in adsp_audio_unsigned64_event */
#define ADSP_AUDIO_IOCTL_CMD_GET_AUDIO_TIME     0x0108c26c


/* these command structures are used for both STREAM and SESSION ioctls */

struct adsp_set_volume_command {
	struct adsp_command_hdr hdr;
	s32 volume;
} __attribute__ ((packed));
    
struct adsp_set_mute_command {
	struct adsp_command_hdr hdr;
	u32 mute; /* 1 == mute */
} __attribute__ ((packed));



/* ---- audio events ---- */

/* All IOCTL commands generate an event with the IOCTL opcode as the */
/* event id after the IOCTL command has been executed. */

/* This event is generated after a media stream session is opened. */
#define ADSP_AUDIO_EVT_STATUS_OPEN              0x0108c0d6

/* This event is generated after a media stream  session is closed. */
#define ADSP_AUDIO_EVT_STATUS_CLOSE             0x0108c0d7

/* Asyncronous buffer consumption. This event is generated after a */
/* recived  buffer is consumed during rendering or filled during */
/* capture opeartion. */
#define ADSP_AUDIO_EVT_STATUS_BUF_DONE              0x0108c0d8

/* This event is generated when rendering operation is starving for */
/* data. In order to avoid audio loss at the end of a plauback, the */
/* client should wait for this event before issuing the close command. */
#define ADSP_AUDIO_EVT_STATUS_BUF_UNDERRUN          0x0108c0d9

/* This event is generated during capture operation when there are no */
/* buffers available to copy the captured audio data */
#define ADSP_AUDIO_EVT_STATUS_BUF_OVERFLOW          0x0108c0da

/* This asynchronous event is generated as a result of an input */
/* sample rate change and/or channel mode change detected by the */
/* decoder. The event payload data is an array of 2 uint32 */
/* values containing the sample rate in Hz and channel mode. */
#define ADSP_AUDIO_EVT_SR_CM_CHANGE             0x0108d329

struct adsp_event_hdr
{
	u32 evt_handle;     /* DAL common header */
	u32 evt_cookie;
	u32 evt_length;

	u32 src;        /* "source" audio address */
	u32 dst;        /* "destination" audio address */

	u32 event_id;
	u32 response_type;
	u32 seq_number;

	u32 context;        /* opaque to DSP */
	u32 data;

	u32 status;
} __attribute__ ((packed));

#if defined(CONFIG_MACH_HTCLEO)
union adsp_event_data
{
	u32 val32;
	uint8_t data[32];
} __attribute__ ((packed));


struct adsp_audio_event
{
	struct adsp_command_hdr hdr;
	u32 session;
	u32 event_id;
	u32 status;
	u32 datalen;
	union adsp_event_data data;
} __attribute__ ((packed));


struct adsp_audio_dal_event
{
	u32     cb_evt;
	u32     loc_evt;
	u32     size;
	struct adsp_audio_event ae;
}  __attribute__ ((packed));

#else
struct adsp_buffer_event 
{
	struct adsp_event_hdr hdr;
	struct adsp_audio_buffer buffer;
} __attribute__ ((packed));

#endif


/* ---- audio device IDs ---- */

/* Device direction Rx/Tx flag */
#define ADSP_AUDIO_RX_DEVICE        0x00
#define ADSP_AUDIO_TX_DEVICE        0x01

/* Default RX or TX device */
#define ADSP_AUDIO_DEVICE_ID_DEFAULT        0x1081679

/* Source (TX) devices */
#define ADSP_AUDIO_DEVICE_ID_HANDSET_MIC    0x107ac8d
#define ADSP_AUDIO_DEVICE_ID_HEADSET_MIC    0x1081510
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_MIC 0x1081512
#define ADSP_AUDIO_DEVICE_ID_BT_SCO_MIC     0x1081518
#define ADSP_AUDIO_DEVICE_ID_TTY_HEADSET_MIC    0x108151b
#define ADSP_AUDIO_DEVICE_ID_I2S_MIC        0x1089bf3

/* Special loopback pseudo device to be paired with an RX device */
/* with usage ADSP_AUDIO_DEVICE_USAGE_MIXED_PCM_LOOPBACK */
#define ADSP_AUDIO_DEVICE_ID_MIXED_PCM_LOOPBACK_TX  0x1089bf2

/* Sink (RX) devices */
#define ADSP_AUDIO_DEVICE_ID_HANDSET_SPKR           0x107ac88
#define ADSP_AUDIO_DEVICE_ID_HEADSET_SPKR_MONO          0x1081511
#define ADSP_AUDIO_DEVICE_ID_HEADSET_SPKR_STEREO        0x107ac8a
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_MONO            0x1081513
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_MONO_W_MONO_HEADSET     0x108c508
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_MONO_W_STEREO_HEADSET   0x108c894
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_STEREO          0x1081514
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_STEREO_W_MONO_HEADSET   0x108c895
#define ADSP_AUDIO_DEVICE_ID_SPKR_PHONE_STEREO_W_STEREO_HEADSET 0x108c509
#define ADSP_AUDIO_DEVICE_ID_BT_SCO_SPKR            0x1081519
#define ADSP_AUDIO_DEVICE_ID_TTY_HEADSET_SPKR           0x108151c
#define ADSP_AUDIO_DEVICE_ID_I2S_SPKR               0x1089bf4
#define ADSP_AUDIO_DEVICE_ID_NULL_SINK              0x108e512

/* BT A2DP playback device. */
/* This device must be paired with */
/* ADSP_AUDIO_DEVICE_ID_MIXED_PCM_LOOPBACK_TX using  */
/* ADSP_AUDIO_DEVICE_USAGE_MIXED_PCM_LOOPBACK mode */
#define ADSP_AUDIO_DEVICE_ID_BT_A2DP_SPKR   0x108151a

/* Voice Destination identifier - specifically used for */
/* controlling Voice module from the Device Control Session */
#define ADSP_AUDIO_DEVICE_ID_VOICE      0x0108df3c

/*  Audio device usage types. */
/*  This is a bit mask to determine which topology to use in the */
/* device session */
#define ADSP_AUDIO_DEVICE_CONTEXT_VOICE         0x01
#define ADSP_AUDIO_DEVICE_CONTEXT_PLAYBACK      0x02
#define ADSP_AUDIO_DEVICE_CONTEXT_MIXED_RECORD      0x10
#define ADSP_AUDIO_DEVICE_CONTEXT_RECORD        0x20
#define ADSP_AUDIO_DEVICE_CONTEXT_PCM_LOOPBACK      0x40

/* for EQ */
#define CAD_EQ_INVALID_DATA        0xFFFFFFFF

/* Equalizer filter band types */
#define ADSP_AUDIO_EQUALIZER_TYPE_NONE      0
#define ADSP_AUDIO_EQUALIZER_BASS_BOOST     1
#define ADSP_AUDIO_EQUALIZER_BASS_CUT       2
#define ADSP_AUDIO_EQUALIZER_TREBLE_BOOST   3
#define ADSP_AUDIO_EQUALIZER_TREBLE_CUT     4
#define ADSP_AUDIO_EQUALIZER_BAND_BOOST     5
#define ADSP_AUDIO_EQUALIZER_BAND_CUT       6

struct cad_audio_eq_cfg {
	u32 enable;
	u32 num_bands;
	struct adsp_audio_eq_band eq_bands[ADSP_AUDIO_MAX_EQ_BANDS];
} __attribute__ ((packed));

int q6audio_set_stream_eq(struct audio_client *ac, struct cad_audio_eq_cfg *eq_cfg);

#endif
