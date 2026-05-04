/* shared/vc1zztab.h: SMPTE VC-1 data table declarations */

#ifndef SHARED_VC1ZZTAB_H
#define SHARED_VC1ZZTAB_H

#ifdef cplusplus
extern"C" {
#endif

 /* Table 226 */
extern const BYTE8 vc1_Inv_Intra_Normal_Scan [64];
 /* Table 227 */
extern const BYTE8 vc1_Inv_Intra_Horizontal_Scan [64];
 /* Table 228 */
extern const BYTE8 vc1_Inv_Intra_Vertical_Scan [64];
 
/* Table 229 */
extern const BYTE8 vc1_Inv_Inter_8x8_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile [64];
 /* Table 230 */
extern const BYTE8 vc1_Inv_Inter_8x4_Scan_Simple_Main_Profiles [32];
 /* Table 231 */
extern const BYTE8 vc1_Inv_Inter_4x8_Scan_Simple_Main_Profiles [32];
 
/* Table 232 */
extern const BYTE8 vc1_Inv_Inter_4x4_Scan_Simple_Main_Profiles_Progressive_Advanced_Profile [16];
 /* Table 233 */
extern const BYTE8 vc1_Inv_Progressive_Inter_8x4_Scan_Advanced_Profile [32];
 /* Table 234 */
extern const BYTE8 vc1_Inv_Progressive_Inter_4x8_Scan_Advanced_Profile [32];
 /* Table 235 */
extern const BYTE8 vc1_Inv_Interlace_Inter_8x8_Scan_Advanced_Profile [64];
 /* Table 236 */
extern const BYTE8 vc1_Inv_Interlace_Inter_8x4_Scan_Advanced_Profile [32];
 /* Table 237 */
extern const BYTE8 vc1_Inv_Interlace_Inter_4x8_Scan_Advanced_Profile [32];
 /* Table 238 */
extern const BYTE8 vc1_Inv_Interlace_Inter_4x4_Scan_Advanced_Profile [16];

#ifdef cplusplus
}
#endif

#endif  /* ndef SHARED_VC1ZZTAB_H */

/* End of shared/vc1zztab.h */

