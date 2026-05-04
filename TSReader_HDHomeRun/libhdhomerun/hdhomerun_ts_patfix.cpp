/*
 * hdhomerun_ts_patfix.c
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
#include "hdhomerun_ts_patfix.h"

static uint8_t hdhomerun_ts_patfix_pat_header[8] = {
	/* PAT header for a single entry table. */
	0x00, 0xb0, 0x0D, 0x01, 0x83, 0xc5, 0x00, 0x00,
};

static void hdhomerun_ts_patfix_internal(uint8_t *data, uint16_t program_number, uint16_t pmt_pid)
{
	uint8_t *ptr = data;
	uint8_t continuity_counter = bit_extract8(ptr[3], 3, 0);

	/* TS Header. */
	ptr[0] = 0x47;
	ptr[1] = 0x60;
	ptr[2] = 0x00;
	ptr[3] = 0x10 | continuity_counter;
	ptr += 4;

	/* Payload unit start offset. */
	ptr[0] = 0x00;
	ptr += 1;

	/* PAT header. */
	memcpy(ptr, hdhomerun_ts_patfix_pat_header, sizeof(hdhomerun_ts_patfix_pat_header));
	ptr += sizeof(hdhomerun_ts_patfix_pat_header);

	/* PAT entry for program. */
	ptr[0] = program_number >> 8;
	ptr[1] = program_number >> 0;
	ptr[2] = 0xE0 | (pmt_pid >> 8);
	ptr[3] = pmt_pid >> 0;
	ptr += 4;

	/* CRC. */
	uint32_t crc = mpeg2_crc_calculate(data + 5, ptr);
	ptr[0] = (uint8_t)(crc >> 24);
	ptr[1] = (uint8_t)(crc >> 16);
	ptr[2] = (uint8_t)(crc >> 8);
	ptr[3] = (uint8_t)(crc >> 0);
	ptr += 4;

	/* Padding. */
	memset(ptr, 0xFF, data + TS_PACKET_SIZE - ptr);
}

void hdhomerun_ts_patfix(uint8_t *data, size_t length, uint16_t program_number, uint16_t pmt_pid)
{
	uint8_t *ptr = data;
	uint8_t *end = data + length;

	while ((ptr + TS_PACKET_SIZE) <= end) {
		uint16_t pid;
		pid  = (uint16_t)bit_extract8(ptr[1], 4, 0) << 8;
		pid |= (uint16_t)ptr[2];
		if (pid != 0x0000) {
			ptr += TS_PACKET_SIZE;
			continue;
		}

		hdhomerun_ts_patfix_internal(ptr, program_number, pmt_pid);
		ptr += TS_PACKET_SIZE;
	}
}

