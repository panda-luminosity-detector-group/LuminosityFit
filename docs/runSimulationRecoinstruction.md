# The `runSimulationReconstruction.py` Script

Generates MC simulation data and performs the entire reconstruction chain. It needs a simulation parameters config file (simparams.conf) and a reconstruction parameters config file (recoparams.conf) to work. Examples of these files can be created with the script `create_sim_reco_pars.py`. Both are human-readable json files. The simulation is then:

```bash
./runSimulationReconstruction.py simparams.conf recoparams.conf
```