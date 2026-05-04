/*
 * hdhomerun_ts_program.c
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
#include "hdhomerun_video.h"
#include "hdhomerun_ts_demux.h"
#include "hdhomerun_ts_program.h"

//#define log_printf(...)
#define log_printf printf

static void hdhomerun_ts_program_pmt_data(void *arg, uint8_t *ptr, size_t length);

static struct hdhomerun_ts_program_entry_t *hdhomerun_ts_program_entry_create(struct hdhomerun_ts_program_list_t *pl, uint16_t program_number)
{
	if (program_number == 0) {
		/* Ignore analog program number in PAT. */
		return NULL;
	}
	if (program_number == 65535) {
		/* Ignore analog program number in VCT. */
		return NULL;
	}

	struct hdhomerun_ts_program_entry_t *entry = (struct hdhomerun_ts_program_entry_t *)calloc(1, sizeof(struct hdhomerun_ts_program_entry_t));
	if (!entry) {
		return NULL;
	}

	entry->parent = pl;
	entry->program_number = program_number;

	struct hdhomerun_ts_program_entry_t **pprev = &pl->items;
	struct hdhomerun_ts_program_entry_t *p = pl->items;
	while (p) {
		if (p->program_number > program_number) {
			break;
		}
		pprev = &p->next;
		p = p->next;
	}
	entry->next = p;
	*pprev = entry;

	return entry;
}

static void hdhomerun_ts_program_entry_add_pid(struct hdhomerun_ts_program_entry_t *entry, uint16_t pid)
{
	unsigned int i;
	for (i = 0; i < entry->pid_count; i++) {
		if (entry->pids[i] == pid) {
			return;
		}
	}

	if (entry->pid_count >= HDHOMERUN_TS_PROGRAM_ENTRY_MAX_PIDS) {
		return;
	}

	log_printf("program %u: adding pid %04x\n", (unsigned int)entry->program_number, (unsigned int)pid);
	entry->pids[entry->pid_count++] = pid;
}

struct hdhomerun_ts_program_entry_t *hdhomerun_ts_program_entry_find_by_program_number(struct hdhomerun_ts_program_list_t *pl, uint16_t program_number)
{
	struct hdhomerun_ts_program_entry_t *entry = pl->items;
	while (entry) {
		if (entry->program_number == program_number) {
			break;
		}
		entry = entry->next;
	}
	return entry;
}

static void hdhomerun_ts_program_psip_vct_data_internal(struct hdhomerun_ts_program_list_t *pl, uint8_t *ptr)
{
	uint16_t program_number;
	program_number  = (uint16_t)ptr[24] << 8;
	program_number |= (uint16_t)ptr[25];

	struct hdhomerun_ts_program_entry_t *entry = hdhomerun_ts_program_entry_find_by_program_number(pl, program_number);
	if (!entry) {
		entry = hdhomerun_ts_program_entry_create(pl, program_number);
		if (!entry) {
			return;
		}
	}

	unsigned int i;
	for (i = 0; i < 7; i++) {
		entry->short_name[i]  = (wchar_t)ptr[i * 2 + 0] << 8;
		entry->short_name[i] |= (wchar_t)ptr[i * 2 + 1];
	}

	entry->major_channel_number  = (uint16_t)bit_extract8(ptr[14], 3, 0) << 6;
	entry->major_channel_number |= (uint16_t)bit_extract8(ptr[15], 7, 2);
	entry->minor_channel_number  = (uint16_t)bit_extract8(ptr[15], 1, 0) << 8;
	entry->minor_channel_number |= (uint16_t)ptr[16];

	log_printf("program %u = %u.%u = %S\n",
		(unsigned int)program_number,
		(unsigned int)entry->major_channel_number,
		(unsigned int)entry->minor_channel_number,
		entry->short_name
	);
}

static void hdhomerun_ts_program_psip_vct_data(struct hdhomerun_ts_program_list_t *pl, uint8_t *ptr, size_t length)
{
	uint8_t *end = ptr + length - 4;

	if ((length < 16) || (length > 1024)) {
		log_printf("invalid TVCT table length = %lu\n", (unsigned long)length);
		return;
	}

	bool_t current_next_indicator = (bool_t)bit_extract8(ptr[5], 0, 0);
	if (!current_next_indicator) {
		log_printf("future TVCT table - ignoring\n");
		return;
	}

	unsigned int num_channels_in_section = (unsigned int)ptr[9];
	ptr += 10;

	unsigned int channel_index;
	for (channel_index = 0; channel_index < num_channels_in_section; channel_index++) {
		if (ptr + 32 > end) {
			return;
		}

		hdhomerun_ts_program_psip_vct_data_internal(pl, ptr);

		unsigned int descriptors_length;
		descriptors_length  = (uint16_t)bit_extract8(ptr[30], 1, 0) << 8;
		descriptors_length |= (uint16_t)ptr[31];

		ptr += 32;
		if (ptr + descriptors_length > end) {
			log_printf("descriptors_length error\n");
			return;
		}
		ptr += descriptors_length;
	}
}

static void hdhomerun_ts_program_psip_data(void *arg, uint8_t *ptr, size_t length)
{
	struct hdhomerun_ts_program_list_t *pl = (struct hdhomerun_ts_program_list_t *)arg;

	uint8_t table_id = ptr[0];
	switch (table_id) {
	case 0xC8:
	case 0xC9:
		hdhomerun_ts_program_psip_vct_data(pl, ptr, length);
		break;
	default:
		break;
	}
}

static void hdhomerun_ts_program_es_data(void *arg, uint8_t *ptr, size_t length)
{
	struct hdhomerun_ts_program_entry_t *entry = (struct hdhomerun_ts_program_entry_t *)arg;

	entry->data_present = TRUE;

	uint8_t scrambling_control = bit_extract8(ptr[3], 7, 6);
	if (scrambling_control != 0) {
		entry->encrypted = TRUE;
	}
}

static void hdhomerun_ts_program_pmt_data(void *arg, uint8_t *ptr, size_t length)
{
	struct hdhomerun_ts_program_entry_t *entry = (struct hdhomerun_ts_program_entry_t *)arg;
	struct hdhomerun_ts_program_list_t *pl = entry->parent;
	uint8_t *end = ptr + length - 4;

	uint8_t table_id = ptr[0];
	if (table_id != 0x02) {
		return;
	}

	if ((length < 16) || (length > 1024)) {
		log_printf("invalid PMT table length = %lu\n", (unsigned long)length);
		return;
	}

	bool_t current_next_indicator = (bool_t)bit_extract8(ptr[5], 0, 0);
	if (!current_next_indicator) {
		log_printf("future PMT table - ignoring\n");
		return;
	}

	uint16_t program_number;
	program_number  = (uint16_t)ptr[3] << 8;
	program_number |= (uint16_t)ptr[4];
	if (program_number != entry->program_number) {
		log_printf("incorrect program number in PMT table - ignoring\n");
		return;
	}

	size_t program_info_length;
	program_info_length  = (size_t)bit_extract8(ptr[10], 3, 0) << 8;
	program_info_length |= (size_t)ptr[11];

	ptr += 12;
	if (ptr + program_info_length > end) {
		log_printf("program_info_length error\n");
		return;
	}
	ptr += program_info_length;

	while (1) {
		if (ptr + 5 > end) {
			return;
		}

		uint16_t es_pid;
		es_pid  = (uint16_t)bit_extract8(ptr[1], 4, 0) << 8;
		es_pid |= (uint16_t)ptr[2];

		size_t es_info_length;
		es_info_length  = (size_t)bit_extract8(ptr[3], 3, 0) << 8;
		es_info_length |= (size_t)ptr[4];

		ptr += 5;
		if (ptr + es_info_length > end) {
			log_printf("es_info_length error\n");
			return;
		}
		ptr += es_info_length;

		hdhomerun_ts_program_entry_add_pid(entry, es_pid);

		struct hdhomerun_ts_demux_t *es_demux = hdhomerun_ts_demux_create(&pl->demux_list, es_pid);
		if (!es_demux) {
			return;
		}
		es_demux->arg = entry;
		es_demux->framer = hdhomerun_ts_demux_framer_raw;
		es_demux->data_callback = hdhomerun_ts_program_es_data;
	}
}

static void hdhomerun_ts_program_pat_data(void *arg, uint8_t *ptr, size_t length)
{
	struct hdhomerun_ts_program_list_t *pl = (struct hdhomerun_ts_program_list_t *)arg;
	uint8_t *end = ptr + length - 4;

	uint8_t table_id = ptr[0];
	if (table_id != 0x00) {
		return;
	}

	if ((length < 12) || (length > 1024)) {
		log_printf("invalid PAT table length = %lu\n", (unsigned long)length);
		return;
	}

	bool_t current_next_indicator = bit_extract8(ptr[5], 0, 0);
	if (!current_next_indicator) {
		log_printf("future PAT table - ignoring\n");
		return;
	}

	ptr += 8;
	while (1) {
		if (ptr + 4 > end) {
			return;
		}
	
		uint16_t program_number;
		program_number  = (uint16_t)ptr[0] << 8;
		program_number |= (uint16_t)ptr[1];

		struct hdhomerun_ts_program_entry_t *entry = hdhomerun_ts_program_entry_find_by_program_number(pl, program_number);
		if (!entry) {
			entry = hdhomerun_ts_program_entry_create(pl, program_number);
			if (!entry) {
				ptr += 4;
				continue;
			}
		}

		uint16_t pmt_pid;
		pmt_pid = (uint16_t)bit_extract8(ptr[2], 4, 0) << 8;
		pmt_pid |= (uint16_t)ptr[3];

		if (entry->pid_count == 0) {
			log_printf("program %u = pid %04x\n", (unsigned int)program_number, (unsigned int)pmt_pid);
			hdhomerun_ts_program_entry_add_pid(entry, pmt_pid);

			struct hdhomerun_ts_demux_t *pmt_demux = hdhomerun_ts_demux_create(&pl->demux_list, pmt_pid);
			if (!pmt_demux) {
				return;
			}
			pmt_demux->arg = entry;
			pmt_demux->framer = hdhomerun_ts_demux_framer_table;
			pmt_demux->data_callback = hdhomerun_ts_program_pmt_data;
		}

		if (entry->pids[0] != pmt_pid) {
			log_printf("program %u PMT pid changed from %04x to %04x\n", (unsigned int)program_number, (unsigned int)entry->pids[0], (unsigned int)pmt_pid);
		}

		ptr += 4;
	}
}

void hdhomerun_ts_program_list_process(struct hdhomerun_ts_program_list_t *pl, uint8_t *data, size_t length)
{
	hdhomerun_ts_demux_process(pl->demux_list, data, length);
}

void hdhomerun_ts_program_list_destroy(struct hdhomerun_ts_program_list_t *pl)
{
	while (pl->demux_list) {
		hdhomerun_ts_demux_destroy(&pl->demux_list, pl->demux_list);
	}

	while (pl->items) {
		struct hdhomerun_ts_program_entry_t *entry = pl->items;
		pl->items = entry->next;
		free(entry);
	}

	free(pl);
}

struct hdhomerun_ts_program_list_t *hdhomerun_ts_program_list_create(void)
{
	/* Create object. */
	struct hdhomerun_ts_program_list_t *pl = (struct hdhomerun_ts_program_list_t *)calloc(1, sizeof(struct hdhomerun_ts_program_list_t));
	if (!pl) {
		return NULL;
	}

	struct hdhomerun_ts_demux_t *pat_demux = hdhomerun_ts_demux_create(&pl->demux_list, 0x0000);
	if (!pat_demux) {
		hdhomerun_ts_program_list_destroy(pl);
		return NULL;
	}
	pat_demux->arg = pl;
	pat_demux->framer = hdhomerun_ts_demux_framer_table;
	pat_demux->data_callback = hdhomerun_ts_program_pat_data;

	struct hdhomerun_ts_demux_t *psip_demux = hdhomerun_ts_demux_create(&pl->demux_list, 0x1FFB);
	if (!psip_demux) {
		hdhomerun_ts_program_list_destroy(pl);
		return NULL;
	}
	psip_demux->arg = pl;
	psip_demux->framer = hdhomerun_ts_demux_framer_table;
	psip_demux->data_callback = hdhomerun_ts_program_psip_data;

	/* Success. */
	return pl;
}

