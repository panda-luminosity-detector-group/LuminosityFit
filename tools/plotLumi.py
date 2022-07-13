#!/usr/bin/env python3

import json
import subprocess
from pathlib import Path

pathSeg0 = "/mnt/work/himsterData/LumiFit/plab_"
pathSeg1 = "GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/merge_data/"

moms = ("1.5", "4.1", "8.9", "11.9", "15.0")

lumiJsonFilePath = "lumi-values.json"
lumiFitRootFileName = "lmd_fitted_data.root"
lumiDataRootFileName = "lmd_data_1of1.root"

for mom in moms:
    with open(pathSeg0 + mom + pathSeg1 + lumiJsonFilePath, "r") as f:
        print(json.load(f)["relative_deviation_in_percent"])

    # with open(os.devnull, 'wb') as devnull:
        # cmd = f"""root -l -q 'plotLumiHist.C("{pathSeg0 + mom + pathSeg1 + lumiRootFileName}", "{mom}.png")'"""
    # subprocess.check_call(['root', '-l', '-q', f'plotLumiHist.C("{pathSeg0 + mom + pathSeg1 + lumiRootFileName}", "{mom}.png")'], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    subprocess.check_call(['root', '-l', '-q', f'plotLmdData.C("{pathSeg0 + mom + pathSeg1 + lumiDataRootFileName}", "{mom}.png")'], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    # os.system()
    