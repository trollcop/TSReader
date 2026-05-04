/* decoder/vc1dec3dhtab.h: SMPTE VC-1 data table declarations */

#ifndef DECODER_VC1DEC3DHTAB_H
#define DECODER_VC1DEC3DHTAB_H

#ifdef cplusplus
extern"C" {
#endif

 /* Table 170 */
extern const vc1DEC_sVLCCode vc1DEC_High_Mot_Intra_VLC [];
 /* Table 172 */
extern const vc1DEC3DH_sRunLevel vc1DEC_High_Mot_Intra_Run_Level [];
 /* Table 177 */
extern const vc1DEC_sVLCCode vc1DEC_High_Mot_Inter_VLC [];
 /* Table 179 */
extern const vc1DEC3DH_sRunLevel vc1DEC_High_Mot_Inter_Run_Level [];
 /* Table 184 */
extern const vc1DEC_sVLCCode vc1DEC_Low_Mot_Intra_VLC [];
 /* Table 186 */
extern const vc1DEC3DH_sRunLevel vc1DEC_Low_Mot_Intra_Run_Level [];
 /* Table 191 */
extern const vc1DEC_sVLCCode vc1DEC_Low_Mot_Inter_VLC [];
 /* Table 193 */
extern const vc1DEC3DH_sRunLevel vc1DEC_Low_Mot_Inter_Run_Level [];
 /* Table 198 */
extern const vc1DEC_sVLCCode vc1DEC_Mid_Rate_Intra_VLC [];
 /* Table 200 */
extern const vc1DEC3DH_sRunLevel vc1DEC_Mid_Rate_Intra_Run_Level [];
 /* Table 205 */
extern const vc1DEC_sVLCCode vc1DEC_Mid_Rate_Inter_VLC [];
 /* Table 207 */
extern const vc1DEC3DH_sRunLevel vc1DEC_Mid_Rate_Inter_Run_Level [];
 /* Table 212 */
extern const vc1DEC_sVLCCode vc1DEC_High_Rate_Intra_VLC [];
 /* Table 214 */
extern const vc1DEC3DH_sRunLevel vc1DEC_High_Rate_Intra_Run_Level [];
 /* Table 219 */
extern const vc1DEC_sVLCCode vc1DEC_High_Rate_Inter_VLC [];
 /* Table 221 */
extern const vc1DEC3DH_sRunLevel vc1DEC_High_Rate_Inter_Run_Level [];

#ifdef cplusplus
}
#endif

#endif  /* ndef DECODER_VC1DEC3DHTAB_H */

/* End of decoder/vc1dec3dhtab.h */

