#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/major.h>
#include <linux/module.h>
#include "twinhan_si.h"
#include "twinhan_ca.h"
#include "twinhan_linux.h"
#include "brktree_reg.h"
#include "ioctl_twinhan.h"
#include "twinhan_common.h"

extern STRING ca_input;
static int CI_Get_CamState(bktr_ptr_t bktr, P_CAM_STATE pCAM_State);
static int CI_Get_AppInfo(bktr_ptr_t bktr, P_APP_INFO pApp_Info);
static int CI_Set_CA_PMT(bktr_ptr_t bktr, P_CA_PMT pCa_Pmt);

extern int put_ioctl_bytes(caddr_t arg, void *val, int size);
extern int get_ioctl_bytes(void *val, caddr_t arg, int size);
extern int put8820CiMSG(bktr_ptr_t bktr, unsigned char *command);
extern int dstPutCommand(bktr_ptr_t bktr, P_PROTOCOL protocol);
extern int dstCommandReply(bktr_ptr_t bktr, P_PROTOCOL protocol);

int ca_minor = CA_MINOR;
extern bktr_reg_t brooktree[ NBKTR ];

int ca_ioctl(struct inode *ino, struct file* file, unsigned int cmd, unsigned long arg)
{
    DBGMSG(">>ca_ioctl()\n");
    
    unsigned int minor = MINOR(ino->i_rdev);
    int unit = UNIT(minor) - ca_minor;
    //ERRMSG(">>si_ioctl() minor=%d, unit=%d\n", minor, unit);
    bktr_ptr_t bktr;
    bktr = &brooktree[unit]; 
    
    int	ret = 0;
    CA_PMT ca_pmt;
    APP_INFO app_info;
    CAM_STATE cam_state;
    
    switch ( cmd ) {
        case CI_GET_APP_INFO:
            memset((void*)&app_info, 0, sizeof(APP_INFO));
            if (CI_Get_AppInfo(bktr, &app_info)) { 
                return -1;
            }
            if ((ret = put_ioctl_bytes((caddr_t)arg, &app_info, sizeof(APP_INFO)))) 
                return ret;
            break; 
                
        case CI_GET_CAM_STATE:
            memset((void*)&cam_state, 0, sizeof(CAM_STATE));
            if (CI_Get_CamState(bktr, &cam_state)) { 
                return -1;
            }
            if ((ret = put_ioctl_bytes((caddr_t)arg, &cam_state, sizeof(CAM_STATE)))) 
                return ret;
            break; 
            
        case CI_SET_CA_PMT:
            if ( (ret = get_ioctl_bytes(&ca_pmt, (caddr_t)arg, sizeof(CA_PMT)))) 
                return ret;
            if (CI_Set_CA_PMT(bktr, &ca_pmt)) {
                ERRMSG("Parser CI_SET_CA_PMT fail !! \n");
                return -1;
            }
            break;
    }            
        
    DBGMSG("<<ca_ioctl()\n");
    return 0;
}    


int ca_open(struct inode* ino, struct file* filep)
{
    DBGMSG(">>ca_open()\n");
        
    MOD_INC_USE_COUNT;
    
    DBGMSG("<<ca_open()\n");
    return 0;
}

int ca_close(struct inode* ino, struct file* filep)
{
    DBGMSG(">>ca_close()\n");
      
    MOD_DEC_USE_COUNT;
    
    DBGMSG("<<ca_close()\n");
    return 0;
}

int CI_Get_CamState(bktr_ptr_t bktr, P_CAM_STATE pCAM_State)
{
    PROTOCOL comm = {0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb};
    PROTOCOL result = {0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb};
    unsigned int Temp;
    
    if (dstPutCommand(bktr, &comm) != 0)
        return -1;
        
    dstCommandReply(bktr, &result);      
    
    Temp = result[4];
	if (Temp == 0xff) {
		pCAM_State->CAM_exist_flag = NON_CI_INFO;
		pCAM_State->MMI_info_flag = NON_CI_INFO;
		return 0;
	}

	if (Temp & 0x80)
		pCAM_State->CAM_exist_flag = ME0;
	else if (Temp & 0x40)
		pCAM_State->CAM_exist_flag = ME1;
	else
		pCAM_State->CAM_exist_flag = NON_CI_INFO;

	if (Temp & 0x20)
		pCAM_State->MMI_info_flag = MMI0;
	else if (Temp & 0x10)
		pCAM_State->MMI_info_flag = MMI1;
	else if (Temp & 0x08)
		pCAM_State->MMI_info_flag = MMI0_ClOSE;
	else if (Temp & 0x04)
		pCAM_State->MMI_info_flag = MMI1_CLOSE;
	else
		pCAM_State->MMI_info_flag = NON_CI_INFO;         
	
    //ERRMSG("CAM State: CAM_Exist_Flag=%d, MMI_Info_Flag=%d \n", pCAM_State->CAM_exist_flag, pCAM_State->MMI_info_flag);
	    
    return 0;
}

int CI_Get_AppInfo(bktr_ptr_t bktr, P_APP_INFO pApp_Info)
{
    STRING comm = {0x07, 0x40, 0x00, 0x00, PCMSG_APPLICATION_INFO, 0x00, 0x00, 0xb8};
    
    if (put8820CiMSG(bktr, comm) != 0) {
        ERRMSG("CI_Get_AppInfo => put8820CiMSG error!!\n");                
        return -1;
    }
    
    pApp_Info->app_type = ca_input[7];
	pApp_Info->application_manufacture = (ca_input[8] << 8) | ca_input[9];
	pApp_Info->manufacture_code = (ca_input[10] << 8) | ca_input[11];
	strcpy(pApp_Info->application_info,(char *)(&(ca_input[12])));
	//ERRMSG("CAM APP_Info: app_type=%d, application_manufacture=%d, manufacture_code=%d, application_info=%s \n", 
	         //pApp_Info->app_type, pApp_Info->application_manufacture, pApp_Info->manufacture_code, pApp_Info->application_info);
    
    return 0;
}

int CI_Set_CA_PMT(bktr_ptr_t bktr, P_CA_PMT pCa_Pmt)
{
    unsigned char *pBuffer = pCa_Pmt->CA_PMT_Buf;
    int nSize = pCa_Pmt->nCA_PMT_Size;
    int i = 0;
    
    if (nSize) {
//ERRMSG("CI_Set_CA_PMT %d, 0x%02x, 0x%02x, 0x%02x\n", nSize, pBuffer[0], pBuffer[1], pBuffer[nSize-1]);
        for (i = (nSize - 1); i >= 0; i--) {
			pBuffer[i+7] = pBuffer[i];
		}
		nSize += 8;		
		pBuffer[0] = nSize - 1;
		pBuffer[1] = 0x40;
		pBuffer[2] = 0x03;
		pBuffer[3] = 0;
		pBuffer[4] = 0x03;
		pBuffer[5] = nSize - 8;
		pBuffer[6] = 0;		
		for (i = 0; i < (nSize - 1); i++) {
			pBuffer[nSize - 1] += pBuffer[i];
		}
		pBuffer[nSize - 1] = ~pBuffer[nSize - 1] + 1;
		
		if (put8820CiMSG(bktr, pBuffer) != 0) {
            ERRMSG("CI_Set_CA_PMT => put8820CiMSG error!!\n");                
            return -1;
        }
		return 0;
    }

    return -1;
}

//*****************************************************************************************************************//
