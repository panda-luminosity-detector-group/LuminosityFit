# doMultipleLuminosityFits.py

Needs a `--forced_resAcc_gen_data` path and a path to the firconfig (has no parameter, just the path). So it's `python doMultipleLuminosityFits.py --forced_resAcc_gen_data path fitconfig.json`. A full working example is here:

```bash
python doMultipleLuminosityFits.py --forced_resAcc_gen_data /lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction /lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_1.5GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction xy_m_cut_real /home/roklasen/LuminosityFit/fitconfig-fast.json
```
