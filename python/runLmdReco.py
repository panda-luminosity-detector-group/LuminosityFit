import json
import os

#read json file
with open('../simconfig.json','r') as simconfig:
        data = simconfig.read()

#parse json file
obj = json.loads(data)


class SimulationParamteres():

    def _init_(self, macropath = ${VMCWORKDIR}"/macro/detector/lmd", pathname, scriptpath = $pwd, random_seed, dirname, workpathname = "lokalscratch/" + ${SLURM_JOB_ID} + "/" + dirname, gen_filepath = workpathname + "/gen_mc.root", XThetaCut, YPhiCut, CleanSig)
        self.macropath = ""
        self.pathname = ""
        self.scriptpath = ""
        self.random_seed: int = $((${random_seed}+${filename_index}))
        self.dirname = ""
        self.workpathname = ""
        self.gen_filepath = ""

if ! -d pathname
 os.system("mkdir -p pathname")

verbosityLevel: int = 0
start_evt: int = $((${num_evts}*${filename_index}))

#switch on "missing plane" search algorithm
misspl = True

#use cuts during trk seacrh with "CA". Should be 'false' if sensors missaligned!
trkcut = True
 if alignment_matrices_path = ""
   trkcut = False

#merge hits on sensors from different sides. true=yes
mergedHits = True
## BOX cut before back-propagation
BoxCut = False
## Write all MC info in TrkQA array
WrAllMC = True

radLength=0.32

prefilter = False
if XThetaCut = True or YPhiCut = True
  prefilter = True

check_stage_success "workpathname + "/Lumi_MC_${start_evt}.root""
if 0 -eq "$?" or  1 -eq "force_level"
  os.system("root -l -b -q 'runLumiPixel2Reco.C('${num_evts}','${start_evt}',"'${workpathname}'", "'${alignment_matrices_path}'", "'${misalignment_matrices_path}'", '${use_point_transform_misalignment}', '$verbositylvl')'")


check_stage_success "workpathname + "/Lumi_recoMerged_${start_evt}.root""
if 0 -eq "$?" or  1 -eq "force_level"
  os.system("root -l -b -q 'runLumiPixel2bHitMerge.C('${num_evts}','${start_evt}',"'${workpathname}'",'$verbositylvl')'")
  # copy Lumi_recoMerged_ for module aligner
  os.system("cp $workpathname/Lumi_recoMerged_${start_evt}.root $pathname/Lumi_recoMerged_${start_evt}.root")

os.system("cd scriptpath")
os.system("./runLmdPairFinder.sh")
os.system("./runLmdTrackFinder.sh")

