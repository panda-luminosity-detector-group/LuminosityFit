# How the Luminosity Fit Software Works

The luminosity extraction can only happen if one or multiple Lumi_TrksQA files are available (For now, a Lumi_Params file must also be present, I don't know yet what's in this file. It's created during MC generation step, but we don't have MC data in the actual experiment).

The LumiFit software then performs multiple individual steps. The MC generation and reconstruction **must** happen on a cluster via slurm job, the luminosity fit specific programs may be run on the cluser as well, but they can also be run on a simple workstation.

# Input Data



1. Lumi_TrkQA files are already there
   - Determine IP Position from them
   - read total cross section from file
2. Now, two things happen at the same time (they are independent of each other):
   - With that IP position, apply xy and momentum cuts on tracks. That is a complete reco chain of the test data (either MC data or experiment data)!
     - copy Lumi_Params and Lumi_Digi from previous simulation
     - run reco, recomerge, track find, track fit macros again
     - then, run trackFilter macro (this is the actual cut)
     - then, back propagation and Lumi_TrkQA again
   - The second thing is resolution and acceptance simulation, so have a realistic detector model
     - this is done with the BOX generator, not the DPM
     - but basically the same, complete MC data generation
     - and reconstruction WITH cuts (it's gotta be realistic)
     - which results in Lumi_TrksQA (but stored elsewhere, only relevant for resolution and acceptance!)
3. the script waits if all jobs are finished and merges the Lumi_TrkQA files from the Box gen sample, to merge the acc and res files. these are important for the LumiFit.
