/*
 * hdhomerun_ts_program.h
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
#ifdef __cplusplus
extern "C" {
#endif

struct hdhomerun_ts_program_entry_t;
struct hdhomerun_ts_program_list_t;

#define HDHOMERUN_TS_PROGRAM_ENTRY_MAX_PIDS 16

struct hdhomerun_ts_program_entry_t {
	struct hdhomerun_ts_program_entry_t *next;
	struct hdhomerun_ts_program_list_t *parent;
	uint16_t program_number;
	uint16_t pids[HDHOMERUN_TS_PROGRAM_ENTRY_MAX_PIDS];
	unsigned int pid_count;
	wchar_t short_name[8];
	uint16_t major_channel_number;
	uint16_t minor_channel_number;
	bool_t encrypted;
	bool_t data_present;
};

struct hdhomerun_ts_program_list_t {
	struct hdhomerun_ts_program_entry_t *items;
	struct hdhomerun_ts_demux_t *demux_list;
};

extern struct hdhomerun_ts_program_list_t *hdhomerun_ts_program_list_create(void);
extern void hdhomerun_ts_program_list_destroy(struct hdhomerun_ts_program_list_t *pl);
extern void hdhomerun_ts_program_list_process(struct hdhomerun_ts_program_list_t *pl, uint8_t *data, size_t length);
extern struct hdhomerun_ts_program_entry_t *hdhomerun_ts_program_entry_find_by_program_number(struct hdhomerun_ts_program_list_t *pl, uint16_t program_number);

#ifdef __cplusplus
}
#endif
