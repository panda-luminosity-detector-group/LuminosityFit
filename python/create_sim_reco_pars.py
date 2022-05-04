import attr
from lumifit.general import write_params_to_file
from lumifit.reconstruction import ReconstructionParameters
from lumifit.simulation import SimulationParameters

simpars = SimulationParameters()
recopars = ReconstructionParameters()

write_params_to_file(attr.asdict(simpars), ".", "simparams.config")
write_params_to_file(attr.asdict(recopars), ".", "recoparams.config")

# read_params_dict = load_params_from_file(".", "simparams.config")
# read_params = SimulationParameters(**read_params_dict)

# assert simpars == read_params
