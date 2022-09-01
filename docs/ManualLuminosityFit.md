# Performing the Luminosity Fit "Manually"

The [determineLuminosity.py](scripts/determineLuminosity.md) scripts aims to automate the Lumi Fit once reconstructed Track data is available. But if the script fails, it's good to know how the steps can be done "manually" (that means calling the appropriate scripts and binaries) by hand.

Except for mc data generation and reonstruction, all merge and fit steps can be done on (sufficiently powerful) consumer hardware.

## Prerequisites

- Working LuminosityFit installation (Docker image prederred)
- Access to HPC system (HimsterII tested, Virgo coming soon)
- A set of reconstructed mc data (use [runSimulationReconstruction.py](scripts/runSimulationRecoinstruction.md) to generate this)

## TL;DR

- Have reconstructed data
- Determine IP for cut
- Reconstruct again, with IP cut
- Generate box data for acceptance and resolution
- Perform Fit

Many of these steps have multiple sub-steps, so it's probably best to read the detailled [Very detailled TL;DR](#very-detailled-tldr) as well.

## Very detailled TL;DR

Because the Lumi Fit software is quite complex and performs a lot of steps, [the detailled mode of operation can be found here](HowThisSoftwareWorks.md).

## Individual Steps

- [Performing the Luminosity Fit "Manually"](#performing-the-luminosity-fit-manually)
  - [Prerequisites](#prerequisites)
  - [TL;DR](#tldr)
  - [Very detailled TL;DR](#very-detailled-tldr)
  - [Individual Steps](#individual-steps)
- [Determine IP from Reconstructed data to reconstruct data with IP cut](#determine-ip-from-reconstructed-data-to-reconstruct-data-with-ip-cut)
- [Generate detector resolution and acceptance plots](#generate-detector-resolution-and-acceptance-plots)
  - [Generate Box Data (for res, acc)](#generate-box-data-for-res-acc)
  - [Make Bunch File Lists](#make-bunch-file-lists)
  - [Copy Data Config](#copy-data-config)
  - [Create LMD Fit Data anhand file lists](#create-lmd-fit-data-anhand-file-lists)
  - [Merge LMD Data](#merge-lmd-data)
- [Prepare data from reconstruction with IP cut](#prepare-data-from-reconstruction-with-ip-cut)
  - [Reconstruct mc (or actual measurment) data with IP cut](#reconstruct-mc-or-actual-measurment-data-with-ip-cut)
  - [Create File List Bunches](#create-file-list-bunches)
  - [Copy Dataconfig](#copy-dataconfig)
  - [Create LMD Data from File Lists](#create-lmd-data-from-file-lists)
  - [Merge LMD Data](#merge-lmd-data-1)
- [Fit reco data (with cuts) to angular data with resolution and acceptance considered](#fit-reco-data-with-cuts-to-angular-data-with-resolution-and-acceptance-considered)
  - [Result](#result)

For each sub step a short explanation and the example command is given.

# Determine IP from Reconstructed data to reconstruct data with IP cut

This is done in the `lmdfit:miniApr21p1` Docker container. In this case, the simulation data is mounted from HimsterII to the workstation via sshfs, but you can use smb as well. Pass the mount point to the container via bind mounts. Please be aware that you'll probaly have to adjust the paths in the following commands.

# Generate detector resolution and acceptance plots

## Generate Box Data (for res, acc)

TODO

## Make Bunch File Lists

This finds all track files and saves their location to `filelist_1.txt` to `filelist_N.txt` for $N =\frac{maximum\_number\_of\_files}{files\_per\_bunch}$.

```
python makeMultipleFileListBunches.py --filenamePrefix Lumi_TrksQA_ --files_per_bunch 10 --maximum_number_of_files 100 /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction
```

## Copy Data Config

Create the `binning_300` directory manually and copy data config:

```
cd /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10
mkdir binning_300
cp ~/LuminosityFit/dataconfig_xy.json binning_300/dataconfig.json
```

## Create LMD Fit Data anhand file lists

For details, see [createLmdFitData](apps/createLmdFitData.md)

Create fit data, data types `e` and `r` (efficiency and resolution). These data typed don't depend on the cross section, but the program needs this parameter. So we can just use `-e 1.0`.

This must be done for each `filelist_*.txt`, so ten times in this example:

:attention: Read the commands carefully and substitute your own paths!

```
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_1.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_2.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_3.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_4.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_5.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_6.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_7.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_8.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_9.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0 &&
./createLmdFitData -m 4.06 -t er -c /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json -d /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction -f /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_10.txt -o /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -n 0 -e 1.0
```

The files shoul be just over 100kb for acc data and a few MB for res data:

```
-rw-r--r-- 1 484457 665970 138K Aug 25 16:11 lmd_acc_data_9.root
-rw-r--r-- 1 484457 665970 4.1M Aug 25 16:11 lmd_res_data_1.root
```

## Merge LMD Data

Merge (the `merge_data` directory will be created automatically):

```
./mergeLmdData -p /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -t r -n 1 -s 0 -f "lmd_res_data_\d*.root"
```

```
./mergeLmdData -p /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -t e -n 1 -s 0 -f "lmd_acc_data_\d*.root"
```

# Prepare data from reconstruction with IP cut

This can be done in parallel to the acceptance and resolution simulations.

## Reconstruct mc (or actual measurment) data with IP cut

TODO, use [runSimulationReconstruction.py](scripts/runSimulationRecoinstruction.md).

## Create File List Bunches

Make file lists:

```
python makeMultipleFileListBunches.py --filenamePrefix Lumi_TrksQA_ --files_per_bunch 10 --maximum_number_of_files 100 /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/
```

## Copy Dataconfig

Make binning folder and copy dataconfig:

```
cd /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10
mkdir binning_300
cp ~/LuminosityFit/dataconfig_xy.json binning_300/dataconfig.json
```

## Create LMD Data from File Lists

Create LMD data erstellen (attention! The type is now `a` and we have to use the correct cross section):

:attention: Read the commands carefully and substitute your own paths!

```
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_1.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_2.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_3.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_4.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_5.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_6.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_7.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_8.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_9.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242 &&
./createLmdFitData -m 4.06 -t a \
-c /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/dataconfig.json \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction \
-f /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/filelist_10.txt \
-o /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 \
-n 0 -e 2.73242
```

## Merge LMD Data

Mergen:

```
./mergeLmdData -p /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300 -t a -n 1 -s 0 -f "lmd_data_\d*.root"
```

# Fit reco data (with cuts) to angular data with resolution and acceptance considered

The fit needs a `fitconfig` or `fitconfig-fast`, the merged acc and res data (the `-a` argument), and of course the merged reconstructed data from the measurement (the `-d` argument). The `-m` is for multi threading.

Perform the fit:

```
./runLmdFit \
-c /mnt/work/LuminosityFit/fitconfig-fast.json \
-m 16 \
-d /mnt/work/himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/merge_data \
-a /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/merge_data \
> ../../../himsterData/LumiFit/plab_4.06GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-100_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/m
erge_data/runLmdFit.log
```

## Result

As the very first step, the fit program calculates a "target" luminosity from the cross section and the reconstructed number of events. The cross section should be calculated with the LumiFit programm, because it depends on the assumed events per time. The formula is then very simple:

$L = \frac{N_{events}}{\sigma}$

The `FVAL` should be as close to zero as possible. It is an indicator of how far the fit result is from the calculated start value.

```
FVAL  = -1163.70603413403296
Edm   = 1.42169486556436322e-07
Nfcn  = 63
dpm_angular_1d:luminosity         = 3.42723e+06  +/-  2333.03
dpm_angular_2d:tilt_x     = 1.23794e-05  +/-  1.24148e-06
dpm_angular_2d:tilt_y     = -1.94357e-05         +/-  1.26713e-06
Fit done!
try: 0 finished with fit status: 0
Adding fit result to storage...
fit storage now contains 1 entries!
adding resolution to pool...
copy constructor: /mnt/work/himsterData/LumiFit/plab_4.06GeV/box_theta_2.6900000000000004-13.01mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0
_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/1-500_xy_m_cut_real/no_alignment_correction/bunches_10/binning_300/merge_data/lmd_res_data_
1of1.root
adding resolution to pool...
available acceptances: 1
available resolutions: 1
elastic data bundle: #theta_{x}^{Rec}
used acceptances: 1
used resolutions: 1
Saving data....

Successfully saved data!
duration: 0.1335 min
Luminosity Fit Result:
measured luminosity:3.42723e+06
measured luminosity error:2333.03
generated luminosity:3.44017e+06
relative deviation (%):-0.376233
relative deviation error (%):0.0678172
```
