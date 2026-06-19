#ifndef CSA_H
#define CSA_H

// Standard CSA.dll
typedef struct tag_CSAKey
{
	int odd_kk[58], even_kk[58];
	unsigned char odd_ck[8], even_ck[8];
} csakey;

typedef void (* td_ptr_set_cws) (unsigned char *cws, csakey *key);
typedef void (* td_ptr_decrypt) (csakey *key, unsigned char *data);
extern void (*ptr_set_cws) (unsigned char *cws, csakey *key);
extern void (*ptr_decrypt) (csakey *key, unsigned char *data);
#ifdef INCLUDE_CSA
void set_cws(unsigned char *cws, csakey *key);
void decrypt(csakey *key, unsigned char *data);
#endif INCLUDE_CSA


// Fast CSA
typedef int (__stdcall * td_get_keyset_size) (void);
typedef int (__stdcall * td_get_internal_parallelism) (void);
typedef void (__stdcall * td_set_control_words) (unsigned char *ev, unsigned char *od, unsigned char *myKeys);
typedef int (__stdcall * td_decrypt_packets)(unsigned char **cluster, unsigned char *myKeys);
extern int (__stdcall *get_keyset_size) (void);
extern int (__stdcall *get_internal_parallelism) (void);
extern void (__stdcall *set_control_words) (unsigned char *ev, unsigned char *od, unsigned char *myKeys);
extern int (__stdcall *decrypt_packets)(unsigned char **cluster, unsigned char *myKeys);

#endif
