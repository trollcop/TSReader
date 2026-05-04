/*
 * hdhomerun_ts_demux.h
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

struct hdhomerun_ts_demux_t;

typedef void (*hdhomerun_ts_demux_framer_func)(struct hdhomerun_ts_demux_t *demux, uint8_t *data);
typedef void (*hdhomerun_ts_demux_data_func)(void *arg, uint8_t *ptr, size_t length);

struct hdhomerun_ts_demux_t {
	struct hdhomerun_ts_demux_t *next;
	uint16_t pid;

	hdhomerun_ts_demux_framer_func framer;
	hdhomerun_ts_demux_data_func data_callback;
	void *arg;

	uint8_t *buffer;
	uint8_t *capacity;
	uint8_t *end;

	uint8_t *payload_unit_start;
};

extern struct hdhomerun_ts_demux_t *hdhomerun_ts_demux_create(struct hdhomerun_ts_demux_t **list, uint16_t pid);
extern void hdhomerun_ts_demux_destroy(struct hdhomerun_ts_demux_t **list, struct hdhomerun_ts_demux_t *demux);
extern void hdhomerun_ts_demux_process(struct hdhomerun_ts_demux_t *list, uint8_t *data, size_t length);
extern struct hdhomerun_ts_demux_t *hdhomerun_ts_demux_find(struct hdhomerun_ts_demux_t *list, uint16_t pid);
extern void hdhomerun_ts_demux_framer_table(struct hdhomerun_ts_demux_t *demux, uint8_t *ptr);
extern void hdhomerun_ts_demux_framer_raw(struct hdhomerun_ts_demux_t *demux, uint8_t *ptr);

static inline uint8_t bit_extract8(uint8_t data, int msb, int lsb)
{
	return (data >> lsb) & ((1 << (msb - lsb + 1)) - 1);
}

extern uint32_t mpeg2_crc_calculate(uint8_t *start, uint8_t *end);

#ifdef __cplusplus
}
#endif

