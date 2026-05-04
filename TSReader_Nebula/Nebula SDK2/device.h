// ------------------------------------------------------------------------------------------------
// ---- This file and its contents are Copyright (C) Nebula Electronics Ltd 2005
// ---- 
// ---- The user may use this file and its contents without restriction, EXCEPT where intended for
// ---- commercial use. If this file or its contents are to be used in a commercial application, then 
// ---- prior written consent must first be obtained from Nebula Electronics Ltd. In this case, please  
// ---- email sales@nebule-electronics.com.
// ---- 
// ---- Although every effort has been made to ensure that this information contained in this file is
// ---- correct, it is supplied WITHOUT WARRANTY. No guarantee of fitness for use or merchantability
// ---- is implied or should be inferred.
// ------------------------------------------------------------------------------------------------

#include <ks.h>

// ------------------------------------------------------------------------------------------------
// ---- Definitions

static GUID DEVINTERFACE_PCI_VIDEO       = {0x6994AD05, 0x93EF, 0x11D0, 0xA3, 0xCC, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96};
static GUID DEVINTERFACE_PCI_AUDIO       = {0xa799a802, 0xa46d, 0x11d0, 0xa1, 0x8c, 0x00, 0xa0, 0x24, 0x01, 0xdc, 0xd4};
static GUID PROPSETID_VIDCAP_CUSTOMBT848 = {0xC44A1A10, 0x0A37, 0x11D2, 0x83, 0x28, 0x00, 0x60, 0x97, 0xBA, 0x83, 0xAB};

#define address                 ULONG
#define data                    DWORD
#define reg_address             ULONG
#define reg_data                DWORD
#define VIDEO                   0                           // PCI Function 0
#define AUDIO                   1                           // PCI Function 1
#define USB                     2                           // USB Function

#define MAX_RISC_INSTR_SIZE     2

#define MAX_LINES               625
#define ACTIVE_LINES            576
#define ACTIVE_PIXELS           768
#define BYTES_PER_PIXEL         3

#define AUDIO_LINES_PER_FIELD   50
#define AUDIO_BYTES_PER_LINE    80
#define VIDEO_BYTES_PER_LINE    (ACTIVE_PIXELS * BYTES_PER_PIXEL)
#define VIDEO_DATA_LENGTH       (ACTIVE_LINES  * VIDEO_BYTES_PER_LINE)
#define NUM_SAMPLES             ((AUDIO_LINES_PER_FIELD * AUDIO_BYTES_PER_LINE))
#define NUM_ANALOG_SAMPLES      (NUM_SAMPLES >> 1)
#define NUM_DIGITAL_SAMPLES     0x0480

#define USB_READ_SIZE           32768                       // Number of bytes transfered per read operation
#define USB_BUFFER_SIZE         (USB_READ_SIZE + 188)
#define PCI_BUFFER_SIZE         NUM_SAMPLES

#define PTS_TIME_CONST          90                          // Represents the 90KHz PTS time constant

// ---- BT878a Registers

#define REG_DSTATUS             0x000
#define REG_E_CROP              0x00c
#define REG_IFORM               0x004
#define REG_O_VDELAY_LO         0x010
#define REG_E_VACTIVE_LO        0x014
#define REG_E_HDELAY_LO         0x018
#define REG_E_HACTIVE_LO        0x01c
#define REG_E_HSCALE_HI         0x020
#define REG_E_HSCALE_LO         0x024
#define REG_E_CONTROL           0x02c
#define REG_E_SCLOOP            0x040
#define REG_WC_UP               0x044
#define REG_OFORM               0x048
#define REG_E_VSCALE_HI         0x04c
#define REG_E_VSCALE_LO         0x050
#define REG_ADELAY              0x060
#define REG_BDELAY              0x064
#define REG_WC_DOWN             0x078
#define REG_SRESET              0x07c
#define REG_TGCTRL              0x084
#define REG_O_CROP              0x08c
#define REG_E_VDELAY_LO         0x090
#define REG_O_VACTIVE_LO        0x094
#define REG_O_HDELAY_LO         0x098
#define REG_O_HACTIVE_LO        0x09c
#define REG_O_HSCALE_HI         0x0a0
#define REG_O_HSCALE_LO         0x0a4
#define REG_O_CONTROL           0x0ac
#define REG_O_SCLOOP            0x0c0
#define REG_O_VSCALE_HI         0x0cc
#define REG_O_VSCALE_LO         0x0d0
#define REG_COLOR_FMT           0x0d4
#define REG_COLOR_CTL           0x0d8
#define REG_CAP_CTL             0x0dc
#define REG_PLL_F_LO            0x0f0
#define REG_PLL_F_HI            0x0f4
#define REG_PLL_XCI             0x0f8
#define REG_INT_STAT            0x100
#define REG_INT_MASK            0x104
#define REG_GPIO_DMA_CTL        0x10c
#define REG_I2C_CON             0x110 
#define REG_AUDIO_PKT_LEN       0x110 
#define REG_RISC_STRT_ADD       0x114
#define REG_GPIO_OUT_EN         0x118
#define REG_GPIO_DATA           0x200

#define RISC_VRE                0x0004
#define RISC_VRO                0x000c
#define RISC_FM1                0x0006
#define RISC_WRITE              0x10000000
#define RISC_JUMP               0x70000000
#define RISC_SYNC               0x80000000
#define RISC_RESET_STATUS       0x000f0000

#define RISC_IRQ                (1<<24)
#define RISC_EOL                (1<<26)
#define RISC_SOL                (1<<27)

// ---- I2C constants

#define I2C_COFDM_ADDR          0x14
#define I2C_TDED_ADDR           0xc2
#define I2C_EEPROM_ADDR         0xa0

#define I2C_THREE_BYTES         (1<<2)
#define I2C_ALLOW_WAIT          (1<<3)
#define I2C_NO_START            (1<<4)
#define I2C_NO_STOP             (1<<5)
#define I2C_RATE_HI             (1<<6)
#define I2C_HW_MODE             (1<<7)
#define I2C_DONE                (1<<8)

#define I2C_FLAGS               (I2C_RATE_HI | I2C_HW_MODE | I2C_ALLOW_WAIT | 0x03)

#define NXT_6000                0           // ATI NXT 6000 COFDM installed
#define MT_352                  1           // Zarlink MT 352 COFDM installed


// ---- Nxtwave 6000 Registers

#define NXT_CHIP_ID             0x00        // Chip identifier (this is a read-only register)
#define NXT_RS_COR_STAT         0x21
#define NXT_VIT_CORE_CTL        0x30
#define NXT_VIT_SYNC_STATUS     0x32
#define NXT_VIT_CORE_INTEN      0x33
#define NXT_VIT_CORE_INTSTAT    0x34
#define NXT_VIT_BERTIME_2       0x38
#define NXT_VIT_BERTIME_1       0x39
#define NXT_VIT_BERTIME_0       0x3a
#define NXT_VIT_BER_1           0x3b
#define NXT_VIT_BER_0           0x3c
#define NXT_OFDM_CORE_CTL       0x40
#define NXT_OFDM_CORE_STAT      0x41
#define NXT_OFDM_CORE_MODEGUARD 0x44
#define NXT_OFDM_AGC_CTL        0x45
#define NXT_AGC_GAIN_1          0x49
#define NXT_AGC_GAIN_2          0x4a
#define NXT_OFDM_ITB_FRQ1       0x4c
#define NXT_OFDM_ITB_FRQ2       0x4d
#define NXT_OFDM_SYR_CTL        0x51
#define NXT_OFDM_SCR_CTL        0x58
#define NXT_OFDM_PPM_CTL1       0x59
#define NXT_OFDM_TRL_RATE1      0x5b
#define NXT_OFDM_TRL_RATE2      0x5c
#define NXT_OFDM_CHC_SNR        0x64
#define NXT_TPS_RCVD2           0x68
#define NXT_TPS_RCVD4           0x6a
#define NXT_ANALOG_CTRL_0       0x80
#define NXT_TUNER_I2C_EN        0x81
#define NXT_DMD_RAQ             0x82
#define NXT_RF_AGC_LSB          0x91
#define NXT_RF_AGC_STAT         0x92
#define NXT_DIAG_CONFIG         0x98
#define NXT_DIAG_CONFIG         0x98
#define NXT_DIAG_MODE           0x99
#define NXT_TS_FORMAT           0x9a

// ---- Zarlink MT352 Registers

#define ZAR_STATUS_0            0x00        // OFDM and TPS status
#define ZAR_STATUS_1            0x01        // FEC status
#define ZAR_STATUS_2            0x02        // Finite state machine (FSM) status
#define ZAR_STATUS_3            0x03        // Tuner status
#define ZAR_STATUS_4            0x04        // Serial 2-wire bus controller status
#define ZAR_INTERRUPT_0         0x05        // OFDM interrupts
#define ZAR_INTERRUPT_1         0x06        // FEC interrupts
#define ZAR_INTERRUPT_2         0x07        // FSM interrupts
#define ZAR_INTERRUPT_3         0x08        // Tuner interrupts
#define ZAR_SNR                 0x09        // Selectable signal-noise-ratio monitor
#define ZAR_VIT_ERR_CNT_2       0x0A        // Viterbi error counter – 24 bit
#define ZAR_VIT_ERR_CNT_1       0x0B        // 
#define ZAR_VIT_ERR_CNT_0       0x0C        // 
#define ZAR_RS_ERR_CNT_2        0x0D        // Reed Solomon error counter – 24 bit 
#define ZAR_RS_ERR_CNT_1        0x0E        // 
#define ZAR_RS_ERR_CNT_0        0x0F        // 
#define ZAR_RS_UBC_1            0x10        // Reed Solomon uncorrected block count – 16 bit 
#define ZAR_RS_UBC_0            0x11        // 
#define ZAR_AGC_GAIN_3          0x12        // AGC RF, IF and total gain – 32 bit
#define ZAR_AGC_GAIN_2          0x13        // 
#define ZAR_AGC_GAIN_1          0x14        // 
#define ZAR_AGC_GAIN_0          0x15        // 
#define ZAR_FREQ_OFFSET_2       0x17        // Frequency offset – 24 bits 
#define ZAR_FREQ_OFFSET_1       0x18        // 
#define ZAR_FREQ_OFFSET_0       0x19        // 
#define ZAR_TIMING_OFFSET_1     0x1A        // Timing recovery loop offset – 16 bits 
#define ZAR_TIMING_OFFSET_0     0x1B        // 
#define ZAR_CHAN_FREQ_1         0x1C        // Channel frequency after search – 16 bits 
#define ZAR_CHAN_FREQ_0         0x1D        // 
#define ZAR_TPS_RECEIVED_1      0x1E        // Received TPS bits – 16 bits 
#define ZAR_TPS_RECEIVED_0      0x1F        // 
#define ZAR_TPS_CURRENT_1       0x20        // TPS bits used by MT352 – 16 bits 
#define ZAR_TPS_CURRENT_0       0x21        //
#define ZAR_TPS_CELL_ID_1       0x22        // TPS cell identifier – 16 bits
#define ZAR_TPS_CELL_ID_0       0x23        // 
#define ZAR_TPS_MISC_DATA_2     0x24        // TPS length, frame number and reserved bits – 24 bits
#define ZAR_TPS_MISC_DATA_1     0x25        // 
#define ZAR_TPS_MISC_DATA_0     0x26        // 
#define ZAR_RESET               0x50        // 0x00
#define ZAR_TPS_GIVEN_1         0x51        // 
#define ZAR_TPS_GIVEN_0         0x52        // 0x80 
#define ZAR_ACQ_CTL             0x53        // Blind acquisition control 0x5F 
#define ZAR_TRL_NOMINAL_RATE_1  0x54        // OFDM channel bandwidth normalised to sampling rate
#define ZAR_TRL_NOMINAL_RATE_0  0x55        // 0x49 
#define ZAR_INPUT_FREQ_1        0x56        // Input signal frequency normalised to sampling rate
#define ZAR_INPUT_FREQ_0        0x57        // 0x05 
#define ZAR_TUNER_ADDR          0x58        // Tuner address 0xC2 
#define ZAR_CHAN_START_1        0x59        // Frequency for acquisition or the Start frequency for a
#define ZAR_CHAN_START_0        0x5A        // 0x00 
#define ZAR_CONT_1              0x5B        // Tuner control data
#define ZAR_CONT_0              0x5C        // 0x00 
#define ZAR_TUNER_GO            0x5D        // Start acquisition or scan (self-resetting) 0x00 
#define ZAR_STATUS_EN_0         0x5F        // Control the status bits connected to the STATUS pin 0x20 
#define ZAR_STATUS_EN_1         0x60        // 0x00
#define ZAR_INTERRUPT_EN_0      0x61        // Control the interrupt bits connected to the IRQ pin 0x00
#define ZAR_INTERRUPT_EN_1      0x62        // 0x00 
#define ZAR_INTERRUPT_EN_2      0x63        // 0x00 
#define ZAR_INTERRUPT_EN_3      0x64        // 0x00
#define ZAR_AGC_TARGET          0x67        // AGC target setting 0x28
#define ZAR_AGC_CTL             0x68        // AGC control parameters 0xA0 
#define ZAR_CAPT_RANGE          0x75        // Frequency capture range control 0x31
#define ZAR_SNR_SELECT_1        0x79        // Carrier number for which SNR is output in SNR register 0x20
#define ZAR_SNR_SELECT_0        0x7A        // 0x00
#define ZAR_RS_ERR_PER_1        0x7C        // Reed-Solomon bit error counting period in units of 1024 204-byte blocks 0x00 
#define ZAR_RS_ERR_PER_0        0x7D        // 0x4D 
#define ZAR_CHIP_ID             0x7F        // Chip identifier (this is a read-only register) 0x13
#define ZAR_CHAN_STOP_1         0x80        // End frequency for a channel scan 0x00
#define ZAR_CHAN_STOP_0         0x81        // 0x00 
#define ZAR_CHAN_STEP_1         0x82        // Step size for a channel scan 0x00 
#define ZAR_CHAN_STEP_0         0x83        // 0x30 
#define ZAR_FEC_LOCK_TIME       0x85        // Time in 32 ms units for FEC lock after OFDM channel found during scan 0x40
#define ZAR_OFDM_LOCK_TIME      0x86        // Time in 8 ms units for finding OFDM channel during scan 0x10
#define ZAR_ACQ_DELAY           0x87        // Delay (ms) to start OFDM acquisition after tuner lock 0x00
#define ZAR_SCAN_CTL            0x88        // Channel scan control 0x0A
#define ZAR_CLOCK_CTL           0x89        // Clock control (not reset by full software reset) 0x30
#define ZAR_CONFIG              0x8A        // Configuration control (not reset by full software reset) 0x48
#define ZAR_MCLK_RATIO          0x8B        // Clock ratio for MPEG-TS output 0x0D
#define ZAR_GPP_CTL             0x8C        // General purpose port control 0x00
#define ZAR_ADC_CTL_1           0x8E        // ADC control 0x00
#define ZAR_ADC_CTL_0           0x8F        // 0x00

// ---- Interrupts

#define INT_HSYNC               (1<<2)
#define INT_OFLOW               (1<<3)
#define INT_REMOTE              (1<<5)
#define INT_GPINT               (1<<9)
#define INT_RISCI               (1<<11)
#define INT_FBUS                (1<<12)
#define INT_FTRGT               (1<<13)
#define INT_FDSR                (1<<14)
#define INT_PPERR               (1<<15)
#define INT_RIPERR              (1<<16)
#define INT_PABORT              (1<<17)
#define INT_OCERR               (1<<18)
#define INT_SCERR               (1<<19)

// ---- GPIO Pin functions

#define CLR_REMOTE              (1<<4)                          // Remote control interrupt enable/clear pin

// ---- Audio/MPEG-II Device driver properties

#define AUDIO_READ_DATA         0
#define AUDIO_AUDIO_DATA        1
#define AUDIO_TS_DATA           2
#define AUDIO_FIFOLEVEL         3
#define AUDIO_SET_ADDR          4
#define AUDIO_WRITE_DATA        5
#define AUDIO_SET_MODE          6
#define AUDIO_SYNC_FIFO         7
#define AUDIO_ERR_COUNT         8

// ---- Video Device driver properties

#define VIDEO_READ_DATA         0
#define VIDEO_GET_RC            1
#define VIDEO_SET_ADDR          2
#define VIDEO_WRITE_DATA        3
#define VIDEO_SET_RC            4
#define VIDEO_GRAB              5
#define VIDEO_SET_RT            6
#define VIDEO_SET_MODE          7
#define VIDEO_GET_END           8
#define VIDEO_SET_END           9

// ---- USB Device driver properties

#define USB_READ_EEPROM         1
#define USB_READ_COFDM          2
#define USB_READ_REMOTE         3
#define USB_WRITE_COFDM         5
#define USB_WRITE_TUNER         6
#define USB_WRITE_REMOTE        7
#define USB_WRITE_REMOTE_TYPE   8
#define USB_DEV_INIT            9

#define USB_WRITE_PIPE          0
#define USB_READ_PIPE           1
#define USB_STREAM_PIPE         2

// ------------------------------------------------------------------------------------------------
// ---- Types

// ------------------------------------------------------------------------------------------------

typedef struct
{
  DWORD     Packet_Error;
  DWORD     FBUS;
  DWORD     FTRGT;
  DWORD     FDSR;
  DWORD     PPERR;
  DWORD     RIPERR;
  DWORD     PABORT;
  DWORD     OCERR;
  DWORD     SCERR;
} errors;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  KSPROPERTY    Property;

  errors        Error_Count;
} prop_err;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  KSPROPERTY    Property;

  BYTE          Image [VIDEO_BYTES_PER_LINE];
} prop_grab;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  KSPROPERTY    Property;

  DWORD         Mode;
  DWORD         Width;
  DWORD         Height;
} prop_mode;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  KSPROPERTY    Property;

  ULONG         Address;
  DWORD         Data;
  DWORD         Size;
} fusion_register;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  BYTE          Command;
  BYTE          Address;
  BYTE          Num_Bytes;
  BYTE          Data        [4];
} usb_register;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  KSPROPERTY    Property;

  DWORD         Read_Pos;
  DWORD         Write_Pos;

  BYTE          Data        [NUM_SAMPLES];
} audio;

// ------------------------------------------------------------------------------------------------

typedef struct
{
  ULONG         Offset;         // INPUT Offset of the packet from the begining of the  buffer.
  ULONG         Length;         // OUTPUT length of data received (for in). OUTPUT 0 for OUT.
  LONG          Status;         // Status code for this packet.     
} ISO_DESC;

// ------------------------------------------------------------------------------------------------
// ---- Prototypes

float       get_tuner_frequency ();                                     // Get the current tuner frequency
reg_data    read_eeprom         (reg_address);                          // Read from the onboard EEPROM
reg_data    read_i2c            (reg_address);                          //
BOOL        read_property       (short, DWORD, void*, int);             // Read a driver custom property
reg_data    read_reg            (short, reg_address);                   //
BOOL        read_usb_data       (void*);                                // Read TS data from the USB driver
DWORD       read_usb_remote     ();                                     // Read the remote control value from the USB device
reg_data    read_usb_i2c        (short, reg_address);                   // Read from the USB i2c bus
void        set_i2c_device      (byte);                                 // Set the I2C destination device's address
void        set_tuner_frequency (float, short);                         // Set the channel on the tuner directly from the frequency
void        wait_i2c_done       ();                                     // Wait until the I2C operation has completed
void        write_i2c           (reg_address, reg_data);                //
void        write_i2c_addr      (reg_address);                          //
void        write_property      (short, DWORD, void*);                  //
void        write_reg           (short, reg_address, reg_data);         //
void        write_tuner         (reg_data);                             // Write to the ALPS tuner using i2c
void        write_usb_i2c       (short, reg_address, reg_data, short);  // Write to the USB i2c bus
void        write_usb_remote    (byte, DWORD);                          // Write a value to the USB remote code variable

