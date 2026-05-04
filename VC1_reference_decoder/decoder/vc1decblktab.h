/* decoder/vc1decblktab.h: SMPTE VC-1 data table declarations */

#ifndef DECODER_VC1DECBLKTAB_H
#define DECODER_VC1DECBLKTAB_H

#ifdef cplusplus
extern"C" {
#endif

 /* Table 166 */
extern const vc1DEC_sVLCCode vc1DEC_Low_Mot_Luminance_DC_Diff_VLC [];
 /* Table 167 */
extern const vc1DEC_sVLCCode vc1DEC_Low_Mot_Chroma_DC_Diff_VLC [];
 /* Table 168 */
extern const vc1DEC_sVLCCode vc1DEC_High_Mot_Luminance_DC_Diff_VLC [];
 /* Table 169 */
extern const vc1DEC_sVLCCode vc1DEC_High_Mot_Chroma_DC_Diff_VLC [];

#ifdef cplusplus
}
#endif

#endif  /* ndef DECODER_VC1DECBLKTAB_H */

/* End of decoder/vc1decblktab.h */

