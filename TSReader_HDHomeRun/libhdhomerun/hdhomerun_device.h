/*
 * hdhomerun_device.h
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

#define HDHOMERUN_DEVICE_MAX_TUNE_TO_LOCK_TIME 1500
#define HDHOMERUN_DEVICE_MAX_LOCK_TO_DATA_TIME 2000
#define HDHOMERUN_DEVICE_MAX_TUNE_TO_DATA_TIME (HDHOMERUN_DEVICE_MAX_TUNE_TO_LOCK_TIME + HDHOMERUN_DEVICE_MAX_LOCK_TO_DATA_TIME)

struct hdhomerun_device_t;

struct hdhomerun_tuner_status_t {
	char channel[32];
	unsigned int signal_strength;
	unsigned int signal_to_noise_quality;
	unsigned int symbol_error_quality;
	uint32_t raw_bits_per_second;
	uint32_t packets_per_second;
};

/*
 * Create a device object.
 *
 * This function will not attempt to connect to the device.
 * The connection will be established when first used.
 *
 * uint32_t device_id = 32-bit device id of device. Set to HDHOMERUN_DEVICE_ID_WILDCARD to match any device ID.
 * uint32_t device_ip = IP address of device. Set to 0 to auto-detect.
 * unsigned int tuner = tuner index (0 or 1). Can be changed later by calling hdhomerun_device_set_tuner.
 *
 * Returns a pointer to the newly created device object.
 *
 * When no longer needed, the socket should be destroyed by calling hdhomerun_device_destroy.
 */
extern struct hdhomerun_device_t *hdhomerun_device_create(uint32_t device_id, uint32_t device_ip, unsigned int tuner);
extern void hdhomerun_device_destroy(struct hdhomerun_device_t *hd);
extern void hdhomerun_device_set_tuner(struct hdhomerun_device_t *hd, unsigned int tuner);

/*
 * Get the local machine IP address used when communicating with the device.
 *
 * This function is useful for determining the IP address to use with set target commands.
 *
 * Returns 32-bit IP address with native endianness, or 0 on error.
 */
extern uint32_t hdhomerun_device_get_local_machine_addr(struct hdhomerun_device_t *hd);

/*
 * Get operations.
 *
 * struct hdhomerun_tuner_status_t *status = Pointer to caller supplied status struct to be populated with result.
 * const char **p<name> = Caller supplied char * to be updated to point to the result string. The string will remain
 *		valid until another call to a device function.
 *
 * Returns 1 if the operation was successful.
 * Returns 0 if the operation was rejected.
 * Returns -1 if a communication error occurred.
 */
extern int hdhomerun_device_get_tuner_status(struct hdhomerun_device_t *hd, struct hdhomerun_tuner_status_t *status);
extern int hdhomerun_device_get_tuner_channel(struct hdhomerun_device_t *hd, char **pchannel);
extern int hdhomerun_device_get_tuner_channelmap(struct hdhomerun_device_t *hd, char **pchannelmap);
extern int hdhomerun_device_get_tuner_filter(struct hdhomerun_device_t *hd, char **pfilter);
extern int hdhomerun_device_get_tuner_target(struct hdhomerun_device_t *hd, char **ptarget);
extern int hdhomerun_device_get_ir_target(struct hdhomerun_device_t *hd, char **ptarget);
extern int hdhomerun_device_get_version(struct hdhomerun_device_t *hd, char **pversion);

/*
 * Set operations.
 *
 * const char *<name> = String to send to device.
 *
 * Returns 1 if the operation was successful.
 * Returns 0 if the operation was rejected.
 * Returns -1 if a communication error occurred.
 */
extern int hdhomerun_device_set_tuner_channel(struct hdhomerun_device_t *hd, const char *channel);
extern int hdhomerun_device_set_tuner_channelmap(struct hdhomerun_device_t *hd, const char *channelmap);
extern int hdhomerun_device_set_tuner_filter(struct hdhomerun_device_t *hd, const char *filter);
extern int hdhomerun_device_set_tuner_target(struct hdhomerun_device_t *hd, char *target);
extern int hdhomerun_device_set_ir_target(struct hdhomerun_device_t *hd, const char *target);

/*
 * Get/set a named control variable on the device.
 *
 * const char *name: The name of var to get/set (c-string). The supported vars is device/firmware dependant.
 * const char *value: The value to set (c-string). The format is device/firmware dependant.

 * char **pvalue: If provided, the caller-supplied char pointer will be populated with a pointer to the value
 *		string returned by the device, or NULL if the device returned an error string. The string will remain
 *		valid until the next call to a control sock function.
 * char **perror: If provided, the caller-supplied char pointer will be populated with a pointer to the error
 *		string returned by the device, or NULL if the device returned an value string. The string will remain
 *		valid until the next call to a control sock function.
 *
 * Returns 1 if the operation was successful (pvalue set, perror NULL).
 * Returns 0 if the operation was rejected (pvalue NULL, perror set).
 * Returns -1 if a communication error occurs.
 */
extern int hdhomerun_device_get_var(struct hdhomerun_device_t *hd, const char *name, char **pvalue, char **perror);
extern int hdhomerun_device_set_var(struct hdhomerun_device_t *hd, const char *name, const char *value, char **pvalue, char **perror);

/*
 * Detect programs on the current channel.
 *
 * The hdhomerun_device_detect_programs_start function initializes the process and tells the device to start
 * streaming data.
 *
 * Returns 1 if the oprtation started successfully.
 * Returns 0 if the operation was rejected.
 * Returns -1 if a communication error occurs.
 *
 * The hdhomerun_device_detect_programs_recv function should be called periodically to process data that has arrived.
 * The buffer can losslessly store 1 second of data, however a more typical call rate would be every 64ms.
 *
 * The hdhomerun_device_detect_programs_stop function tells the device to stop streaming data.
 *
 * The hdhomerun_device_detect_programs_result function returns a pointer to the first entry in the list of program
 * entries that has been discovered to date. It is valid to call this function before or after calling stop. The
 * entry list will remain valid until another program detection or stream operation call. See hdhomerun_ts_program.h
 * for details on the format of the program entry.
 *
 * The hdhomerun_device_detect_programs_find_by_program_number function returns a pointer to the program entry
 * for the given program number as discovered to date. It is valid to call this function before or after calling stop.
 * The entry list will remain valid until another program detection or stream operation call. See hdhomerun_ts_program.h
 * for details on the format of the program entry.
 *
 * The hdhomerun_device_program_detect_filter function returns the filter string for the given program number.
 * The function returns NULL if the program has not been detected yet.
 */
extern int hdhomerun_device_detect_programs_start(struct hdhomerun_device_t *hd);
extern void hdhomerun_device_detect_programs_recv(struct hdhomerun_device_t *hd);
extern void hdhomerun_device_detect_programs_stop(struct hdhomerun_device_t *hd);
extern struct hdhomerun_ts_program_entry_t *hdhomerun_device_detect_programs_result(struct hdhomerun_device_t *hd);
extern struct hdhomerun_ts_program_entry_t *hdhomerun_device_detect_programs_find_by_program_number(struct hdhomerun_device_t *hd, uint16_t program_number);
extern char *hdhomerun_device_program_detect_filter(struct hdhomerun_device_t *hd, uint16_t program_number);

/*
 * Stream a filtered program or the unfiltered stream.
 *
 * The hdhomerun_device_stream_start function initializes the process and tells the device to start streamin data.
 *
 * uint16_t program_number = The program number to filer, or 0 for unfiltered.
 *
 * Returns 1 if the oprtation started successfully.
 * Returns 0 if the operation was rejected.
 * Returns -1 if a communication error occurs.
 *
 * The hdhomerun_device_stream_recv function should be called periodically to receive the stream data.
 * The buffer can losslessly store 1 second of data, however a more typical call rate would be every 64ms.
 *
 * The hdhomerun_device_stream_stop function tells the device to stop streaming data.
 */
extern int hdhomerun_device_stream_start(struct hdhomerun_device_t *hd, uint16_t program_number);
extern uint8_t *hdhomerun_device_stream_recv(struct hdhomerun_device_t *hd, size_t max_size, size_t *pactual_size);
extern void hdhomerun_device_stream_stop(struct hdhomerun_device_t *hd);

/*
 * Upload new firmware to the device.
 *
 * FILE *upgrade_file: File pointer to read from. The file must have been opened in binary mode for reading.
 *
 * Returns 1 if the upload succeeded.
 * Returns 0 if the upload was rejected.
 * Returns -1 if an error occurs.
 */
extern int hdhomerun_device_upgrade(struct hdhomerun_device_t *hd, FILE *upgrade_file);

/*
 * Low level accessor functions. 
 */
extern struct hdhomerun_control_sock_t *hdhomerun_device_get_control_sock(struct hdhomerun_device_t *hd);
extern struct hdhomerun_video_sock_t *hdhomerun_device_get_video_sock(struct hdhomerun_device_t *hd);

#ifdef __cplusplus
}
#endif
