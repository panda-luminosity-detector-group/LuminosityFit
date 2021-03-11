import json

#read json file
with open('../simconfig.json','r') as simconfig:
    data = simconfig.read()

#parse json file
obj = json.loads(data)

class SimulationParamteres():

    def _init_(self, macropath = "${VMCWORKDIR}/macro/detector/lmd", sim, params, here)
        self.macropath = ""
        self.sim = sim
        self.params = params
        self.here = here


