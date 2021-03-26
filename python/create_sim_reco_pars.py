import attr
from lumifit.general import load_params_from_file, write_params_to_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters, SimulationType

simpars = SimulationParameters(SimulationType.PBARP_ELASTIC, 1000, 1, 1.5)
recopars = ReconstructionParameters(1000, 1, 1.5)

write_params_to_file(attr.asdict(simpars), ".", "simparams.config")
write_params_to_file(attr.asdict(recopars), ".", "recoparams.config")

# read_params_dict = load_params_from_file(".", "simparams.config")

# read_params = SimulationParameters(**read_params_dict)

# assert simpars == read_params

# os.system("python runKoaSimReco.py")
# os.system("python runKoaReco.py")
