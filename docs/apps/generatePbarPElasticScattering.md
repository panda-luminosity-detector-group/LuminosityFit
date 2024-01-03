Table of Contents:

- [Generate $\\bar{P}P$ Elastic Scattering](#generate-barpp-elastic-scattering)
  - [Minimum Example](#minimum-example)
  - [Example Output](#example-output)
  - [Why do we need lower and upper bounds at all?](#why-do-we-need-lower-and-upper-bounds-at-all)
  - [To Determine the Luminosity from MC Data](#to-determine-the-luminosity-from-mc-data)

# Generate $\bar{P}P$ Elastic Scattering

This app calculates the $\bar{P}P$ elastic cross section and optionally generates appropriate dpm MC tracks.

For details, see the `-h` flag.

## Minimum Example

The beam momentum and number of events are *always* required. If the number of events is `0`, the total cross section is calculated and stored to `elastic_cross_section.txt`. Lower and upper bounds are optional for the program to work, but they must still be included to get correct cross section values.

```bash
./generatePbarPElasticScattering 8.9 0 -l 2.7 -u 13.0 -o ./sigma-8.9.txt
```

> :warning: It doesn't really matter what file name you specify, i.e. `-o ./sigma-8.9.txt`. The file will always be called `elastic_cross_section.txt` anyway. But the path is respected (and also very important).

## Example Output

```
Generating model ... 
(dpm_angular_1d) WARNING: This model parameter p_lab already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter luminosity already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter E_lab already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter S already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter pcm2 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter gamma already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter beta already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter beta_lab_cms already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter sigma_tot already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter b already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter rho already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter A1 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter A2 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter A3 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter T1 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter T2 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter p_lab already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter luminosity already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter E_lab already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter S already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter pcm2 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter gamma already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter beta already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter beta_lab_cms already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter sigma_tot already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter b already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter rho already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter A1 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter A2 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter A3 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter T1 already exists. Returning existing value reference!
(dpm_angular_1d) WARNING: This model parameter T2 already exists. Returning existing value reference!
Integrated total elastic cross section in theta range [2.7 - 13] -> t [0.000577417 - 0.0133738] is 2.2201 mb
```

The resultant cross section is `2.2201 mb`.

> :warning: If the lower and upper bounds are omitted, the result is `2.33462 mb`. It is very important to generate the DPM data and the cross section with the same parameters.

## Why do we need lower and upper bounds at all?

To save (a little) computation time during MC generation, we can restrict the angular region at which events are generated, since we cant see them with our detector anyway.

However, in the actual Dual Parton Model (DPM), these events are still there and still contribute to the total cross section. At the real experiment with real measurement data, no limits must be set to get the correct cross section.

## To Determine the Luminosity from MC Data

During a lumi fit, the output file **must** be called `elastic_cross_section.txt`, and it **must** be in the base data directory of the scenario. The limits were set in the mc data, so they must be here as well. The call would then look like this:

```bash
./generatePbarPElasticScattering 8.9 0 -l 2.7 -u 13.0 -o /lustre/miifs05/scratch/him-specf/paluma/roklasen/LumiFit/plab_8.90GeV/dpm_elastic_theta_2.7-13.0mrad_recoil_corrected/ip_offset_XYZDXDYDZ_0.0_0.0_0.0_0.0_0.0_0.0/beam_grad_XYDXDY_0.0_0.0_0.0_0.0/no_geo_misalignment/100000/elastic_cross_section.txt
```