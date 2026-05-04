#ifndef __MSG_H__
#define __MSG_H__

#ifdef __cplusplus
extern "C" {
#endif

void SetProgramName (char * name);
const char * GetProgramName();
void ShowProgramName (bool Show);
void Message (char * msg);
void Error (char * msg);

#ifdef __cplusplus
}
#endif

#endif // __MSG_H__
