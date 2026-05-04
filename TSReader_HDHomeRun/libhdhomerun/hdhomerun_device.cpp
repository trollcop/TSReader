/*
 * hdhomerun_record.c
 *
 * Copyright © 2006 Silicondust Engineering Ltd. <www.silicondust.com>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "hdhomerun_os.h"
#include "hdhomerun_pkt.h"
#include "hdhomerun_control.h"
#include "hdhomerun_video.h"
#include "hdhomerun_ts_demux.h"
#include "hdhomerun_ts_program.h"
#include "hdhomerun_ts_patfix.h"
#include "hdhomerun_device.h"

struct hdhomerun_device_t {
	struct hdhomerun_control_sock_t *cs;
	struct hdhomerun_video_sock_t *vs;
	struct hdhomerun_ts_program_list_t *pl;
	unsigned int tuner;
	uint16_t program_number;
	uint16_t program_pmt_pid;
	char result_buffer[1024];
};

struct hdhomerun_device_t *hdhomerun_device_create(uint32_t device_id, uint32_t device_ip, unsigned int tuner)
{
	struct hdhomerun_device_t *hd = (struct hdhomerun_device_t *)calloc(1, sizeof(struct hdhomerun_device_t));
	if (!hd) {
		return NULL;
	}

	hd->tuner = tuner;

	hd->cs = hdhomerun_control_create(device_id, device_ip);
	if (!hd->cs) {
		free(hd);
		return NULL;
	}

	return hd;
}

void hdhomerun_device_destroy(struct hdhomerun_device_t *hd)
{
	if (hd->pl) {
		hdhomerun_ts_program_list_destroy(hd->pl);
	}
	if (hd->vs) {
		hdhomerun_video_destroy(hd->vs);
	}

	hdhomerun_control_destroy(hd->cs);

	free(hd);
}

void hdhomerun_device_set_tuner(struct hdhomerun_device_t *hd, unsigned int tuner)
{
	hd->tuner = tuner;
}

struct hdhomerun_control_sock_t *hdhomerun_device_get_control_sock(struct hdhomerun_device_t *hd)
{
	return hd->cs;
}

struct hdhomerun_video_sock_t *hdhomerun_device_get_video_sock(struct hdhomerun_device_t *hd)
{
	if (!hd->vs) {
		hd->vs = hdhomerun_video_create(0, VIDEO_DATA_BUFFER_SIZE_1S);
	}
	return hd->vs;
}

uint32_t hdhomerun_device_get_local_machine_addr(struct hdhomerun_device_t *hd)
{
	return hdhomerun_control_get_local_addr(hd->cs);
}

static uint32_t hdhomerun_device_get_status_parse(const char *status_str, const char *tag)
{
	const char *ptr = strstr(status_str, tag);
	if (!ptr) {
		return 0;
	}

	unsigned long value = 0;
	sscanf(ptr + strlen(tag), "%lu", &value);

	return (uint32_t)value;
}

int hdhomerun_device_get_tuner_status(struct hdhomerun_device_t *hd, struct hdhomerun_tuner_status_t *status)
{
	memset(status, 0, sizeof(struct hdhomerun_tuner_status_t));

	char name[32];
	sprintf(name, "/tuner%u/status", hd->tuner);

	char *status_str;
	int ret = hdhomerun_control_get(hd->cs, name, &status_str, NULL);
	if (ret <= 0) {
		return ret;
	}

	char *channel = strstr(status_str, "ch=");
	if (channel) {
		sscanf(channel + 3, "%s", status->channel);
	}

	status->signal_strength = (unsigned int)hdhomerun_device_get_status_parse(status_str, "ss=");
	status->signal_to_noise_quality = (unsigned int)hdhomerun_device_get_status_parse(status_str, "snq=");
	status->symbol_error_quality = (unsigned int)hdhomerun_device_get_status_parse(status_str, "seq=");
	status->raw_bits_per_second = hdhomerun_device_get_status_parse(status_str, "bps=");
	status->packets_per_second = hdhomerun_device_get_status_parse(status_str, "pps=");

	return 1;
}

int hdhomerun_device_get_tuner_channel(struct hdhomerun_device_t *hd, char **pchannel)
{
	char name[32];
	sprintf(name, "/tuner%u/channel", hd->tuner);
	return hdhomerun_control_get(hd->cs, name, pchannel, NULL);
}

int hdhomerun_device_get_tuner_channelmap(struct hdhomerun_device_t *hd, char **pchannelmap)
{
	char name[32];
	sprintf(name, "/tuner%u/channelmap", hd->tuner);
	return hdhomerun_control_get(hd->cs, name, pchannelmap, NULL);
}

int hdhomerun_device_get_tuner_filter(struct hdhomerun_device_t *hd, char **pfilter)
{
	char name[32];
	sprintf(name, "/tuner%u/filter", hd->tuner);
	return hdhomerun_control_get(hd->cs, name, pfilter, NULL);
}

int hdhomerun_device_get_tuner_target(struct hdhomerun_device_t *hd, char **ptarget)
{
	char name[32];
	sprintf(name, "/tuner%u/target", hd->tuner);
	return hdhomerun_control_get(hd->cs, name, ptarget, NULL);
}

int hdhomerun_device_get_ir_target(struct hdhomerun_device_t *hd, char **ptarget)
{
	return hdhomerun_control_get(hd->cs, "/ir/target", ptarget, NULL);
}

int hdhomerun_device_get_version(struct hdhomerun_device_t *hd, char **pversion)
{
	return hdhomerun_control_get(hd->cs, "/sys/version", pversion, NULL);
}

int hdhomerun_device_set_tuner_channel(struct hdhomerun_device_t *hd, const char *channel)
{
	char name[32];
	sprintf(name, "/tuner%u/channel", hd->tuner);
	return hdhomerun_control_set(hd->cs, name, channel, NULL, NULL);
}

int hdhomerun_device_set_tuner_channelmap(struct hdhomerun_device_t *hd, const char *channelmap)
{
	char name[32];
	sprintf(name, "/tuner%u/channelmap", hd->tuner);
	return hdhomerun_control_set(hd->cs, name, channelmap, NULL, NULL);
}

int hdhomerun_device_set_tuner_filter(struct hdhomerun_device_t *hd, const char *filter)
{
	char name[32];
	sprintf(name, "/tuner%u/filter", hd->tuner);
	return hdhomerun_control_set(hd->cs, name, filter, NULL, NULL);
}

int hdhomerun_device_set_tuner_target(struct hdhomerun_device_t *hd, char *target)
{
	char name[32];
	sprintf(name, "/tuner%u/target", hd->tuner);
	return hdhomerun_control_set(hd->cs, name, target, NULL, NULL);
}

static int hdhomerun_device_set_tuner_target_to_local(struct hdhomerun_device_t *hd)
{
	char target[64];
	uint32_t local_ip = hdhomerun_control_get_local_addr(hd->cs);
	uint16_t local_port = hdhomerun_video_get_local_port(hd->vs);
	sprintf(target, "%u.%u.%u.%u:%u",
		(unsigned int)(local_ip >> 24) & 0xFF, (unsigned int)(local_ip >> 16) & 0xFF,
		(unsigned int)(local_ip >> 8) & 0xFF, (unsigned int)(local_ip >> 0) & 0xFF,
		(unsigned int)local_port
	);

	return hdhomerun_device_set_tuner_target(hd, target);
}

int hdhomerun_device_set_ir_target(struct hdhomerun_device_t *hd, const char *target)
{
	return hdhomerun_control_set(hd->cs, "/ir/target", target, NULL, NULL);
}

int hdhomerun_device_get_var(struct hdhomerun_device_t *hd, const char *name, char **pvalue, char **perror)
{
	return hdhomerun_control_get(hd->cs, name, pvalue, perror);
}

int hdhomerun_device_set_var(struct hdhomerun_device_t *hd, const char *name, const char *value, char **pvalue, char **perror)
{
	return hdhomerun_control_set(hd->cs, name, value, pvalue, perror);
}

static int hdhomerun_device_video_start(struct hdhomerun_device_t *hd)
{
	/* Create video socket. */
	hdhomerun_device_get_video_sock(hd);
	if (!hd->vs) {
		return -1;
	}

	/* Create program list object. */
	if (hd->pl) {
		hdhomerun_ts_program_list_destroy(hd->pl);
	}
	hd->pl = hdhomerun_ts_program_list_create();
	if (!hd->pl) {
		return -1;
	}

	/* Set filter. */
	int ret = hdhomerun_device_set_tuner_filter(hd, "0x0000-0x1FFF");
	if (ret <= 0) {
		return ret;
	}

	/* Set target. */
	ret = hdhomerun_device_set_tuner_target_to_local(hd);
	if (ret <= 0) {
		return ret;
	}

	/* Flush video buffer. */
	usleep(64000);
	hdhomerun_video_flush(hd->vs);

	/* Success. */
	return 1;
}

static void hdhomerun_device_video_stop(struct hdhomerun_device_t *hd)
{
	hdhomerun_device_set_tuner_target(hd, "none");
}

int hdhomerun_device_detect_programs_start(struct hdhomerun_device_t *hd)
{
	return hdhomerun_device_video_start(hd);
}

void hdhomerun_device_detect_programs_recv(struct hdhomerun_device_t *hd)
{
	size_t actual_size;
	uint8_t *ptr = hdhomerun_video_recv(hd->vs, 0xFFFFFFFF, &actual_size);
	if (!ptr) {
		return;
	}

	hdhomerun_ts_program_list_process(hd->pl, ptr, actual_size);
}

void hdhomerun_device_detect_programs_stop(struct hdhomerun_device_t *hd)
{
	hdhomerun_device_video_stop(hd);
}

struct hdhomerun_ts_program_entry_t *hdhomerun_device_detect_programs_result(struct hdhomerun_device_t *hd)
{
	if (!hd->pl) {
		return NULL;
	}
	return hd->pl->items;
}

struct hdhomerun_ts_program_entry_t *hdhomerun_device_detect_programs_find_by_program_number(struct hdhomerun_device_t *hd, uint16_t program_number)
{
	if (!hd->pl) {
		return NULL;
	}
	return hdhomerun_ts_program_entry_find_by_program_number(hd->pl, program_number);
}

char *hdhomerun_device_program_detect_filter(struct hdhomerun_device_t *hd, uint16_t program_number)
{
	if (program_number == 0) {
		sprintf(hd->result_buffer, "0x0000-0x1FFE");
		return hd->result_buffer;
	}

	if (!hd->pl) {
		return NULL;
	}

	struct hdhomerun_ts_program_entry_t *entry = hdhomerun_ts_program_entry_find_by_program_number(hd->pl, program_number);
	if (!entry) {
		return NULL;
	}
	if (entry->pid_count <= 1) {
		return NULL;
	}

	char *ptr = hd->result_buffer;
	sprintf(ptr, "0x0000");
	ptr = strchr(ptr, 0);

	unsigned int i;
	for (i = 0; i < entry->pid_count; i++) {
		sprintf(ptr, " 0x%04X", entry->pids[i]);
		ptr = strchr(ptr, 0);
	}

	return hd->result_buffer;
}

int hdhomerun_device_stream_start(struct hdhomerun_device_t *hd, uint16_t program_number)
{
	hd->program_number = program_number;
	hd->program_pmt_pid = 0;

	return hdhomerun_device_video_start(hd);
}

static void hdhomerun_device_stream_recv_find_program(struct hdhomerun_device_t *hd, uint8_t *data, size_t length)
{
	/* Program detection. */
	hdhomerun_ts_program_list_process(hd->pl, data, length);

	/* Check if program found. */
	struct hdhomerun_ts_program_entry_t *entry = hdhomerun_ts_program_entry_find_by_program_number(hd->pl, hd->program_number);
	if (!entry) {
		return;
	}
	if (entry->pid_count <= 1) {
		return;
	}

	/* Found! */
	hd->program_pmt_pid = entry->pids[0];

	/* Set filter. */
	char filter[1024];
	sprintf(filter, "0x0000");

	char *ptr = strchr(filter, 0);
	unsigned int i;
	for (i = 0; i < entry->pid_count; i++) {
		sprintf(ptr, " 0x%04X", entry->pids[i]);
		ptr = strchr(ptr, 0);
	}

	hdhomerun_device_set_tuner_filter(hd, filter);

	/* Flush video buffer. */
	usleep(64000);
	hdhomerun_video_flush(hd->vs);
}

uint8_t *hdhomerun_device_stream_recv(struct hdhomerun_device_t *hd, size_t max_size, size_t *pactual_size)
{
	uint8_t *data = hdhomerun_video_recv(hd->vs, max_size, pactual_size);
	if (!data) {
		return NULL;
	}

	if (hd->program_number == 0) {
		return data;
	}

	if (hd->program_pmt_pid == 0) {
		hdhomerun_device_stream_recv_find_program(hd, data, *pactual_size);
		*pactual_size = 0;
		return NULL;
	}

	hdhomerun_ts_patfix(data, *pactual_size, hd->program_number, hd->program_pmt_pid);
	return data;
}

void hdhomerun_device_stream_stop(struct hdhomerun_device_t *hd)
{
	hdhomerun_device_video_stop(hd);
}

int hdhomerun_device_upgrade(struct hdhomerun_device_t *hd, FILE *upgrade_file)
{
	return hdhomerun_control_upgrade(hd->cs, upgrade_file);
}
