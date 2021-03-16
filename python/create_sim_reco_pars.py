import attr
from lumifit.general import load_params_from_file, write_params_to_file
from lumifit.simulation import SimulationParameters, SimulationType

simpars = SimulationParameters(SimulationType.PBARP_ELASTIC, 1000, 1, 1.5)

write_params_to_file(attr.asdict(simpars), ".", "simparams.config")

read_params_dict = load_params_from_file(".", "simparams.config")

read_params = SimulationParameters(**read_params_dict)

assert simpars == read_params
