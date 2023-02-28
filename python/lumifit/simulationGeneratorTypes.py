from enum import Enum


class SimulationGeneratorType(Enum):
    BOX = "box"
    PBARP_ELASTIC = "dpm_elastic"
    PBARP = "dpm_elastic_inelastic"
    NOISE = "noise"
    RESACCBOX = "resAcc-box"
    RESACCPBARP_ELASTIC = "resAcc-dpm_elastic"
