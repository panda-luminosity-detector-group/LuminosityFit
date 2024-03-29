set(INCLUDE_DIRECTORIES
  ${ROOT_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  $ENV{VMCWORKDIR}
)
include_directories(${INCLUDE_DIRECTORIES})

# ########################## active apps ##################################
add_executable(calculateElasticCrossSection calculateElasticCrossSection.cxx)
target_link_libraries(calculateElasticCrossSection LmdUI ROOT::Hist ROOT::Gpad ROOT::RIO)

add_executable(checkDivergenceSmearing checkDivergenceSmearing.cxx)
target_link_libraries(checkDivergenceSmearing LmdUI ROOT::Hist ROOT::RIO)

add_executable(analyzeResolutionMapData analyzeResolutionMapData.cxx)
target_link_libraries(analyzeResolutionMapData LmdUI ROOT::Hist ROOT::Gpad ROOT::RIO ROOT::Graf)

if(PANDAROOT_FOUND)
  add_executable(createLmdFitData createLmdFitData.cxx)
  target_link_libraries(createLmdFitData LmdUI LmdFitPandaDataIO ${BOOST_LIBRARIES})
endif()

if(KOALASOFT_FOUND)
  add_executable(createKoaFitData createKoaFitData.cxx)
  target_link_libraries(createKoaFitData LmdUI LmdFitKoaDataIO ${BOOST_LIBRARIES})
endif()

add_executable(determineBeamOffset determineBeamOffset.cxx)
target_link_libraries(determineBeamOffset PRIVATE LmdUI ${ROOT_LIBRARIES})

add_executable(determineLumiFitSystematics determineLumiFitSystematics.cxx)
target_link_libraries(determineLumiFitSystematics LmdUI ${ROOT_LIBRARIES})

add_executable(makeResolutionPlots makeResolutionPlots.cxx)
target_link_libraries(makeResolutionPlots LmdUI ${ROOT_LIBRARIES})

add_executable(mergeLmdData mergeLmdData.cxx)
target_link_libraries(mergeLmdData LmdUI ${ROOT_LIBRARIES})

add_executable(plotIPDistribution plotIPDistribution.cxx)
target_link_libraries(plotIPDistribution LmdUI ROOT::Graf ROOT::RIO)

add_executable(extractLuminosity extractLuminosityValues.cxx)
target_link_libraries(extractLuminosity LmdUI)

add_executable(plotLumiFitResults plotLumiFitResults.cxx)
target_link_libraries(plotLumiFitResults LmdUI ROOT::Hist ROOT::Gpad ROOT::RIO ROOT::Graf)

add_executable(runLmdFit runLmdFit.cxx)
target_link_libraries(runLmdFit LmdUI)

add_executable(fitRawDPMElasticData fitRawDPMElasticData.cxx)
target_link_libraries(fitRawDPMElasticData LmdUI ROOT::EG ROOT::Gpad ROOT::Hist ROOT::Tree ROOT::Physics ROOT::MathCore ROOT::Graf)

add_executable(plot2DDPMModel plot2DDPMModel.cxx)
target_link_libraries(plot2DDPMModel LmdUI ROOT::Hist ROOT::RIO)

add_executable(generatePbarPElasticScattering generatePbarPElasticScattering.cxx)
target_link_libraries(generatePbarPElasticScattering LmdModel LmdFitData Boost::filesystem ROOT::Tree ROOT::MathCore ROOT::Physics ROOT::EG)

add_executable(checkFramework checkFramework.cxx)
target_link_libraries(checkFramework Model ROOT::Core ROOT::MathCore ROOT::RIO ROOT::Hist Boost::system)

# ########################## inactive apps ##################################

# ########################## create fitRawDPMElasticData app ##################################
add_executable(compareAcceptances compareAcceptances.cxx)
target_link_libraries(compareAcceptances LmdUI)

# add_executable(analyzeAcceptances analyzeAcceptances.cxx)
# target_link_libraries(analyzeAcceptances LmdUI Minuit2)

# add_executable(phiSliceData phiSliceData.cxx)
# target_link_libraries(phiSliceData Lmd LmdTool MathMore Eve)

# ########################## create resolutionModelFitTesting1D app ##################################

# add_executable(resolutionModelFitTesting1D resolutionModelFitTesting1D.cxx)
# target_link_libraries(resolutionModelFitTesting1D LmdFit)

# ########################## create checkTilted2DDPMModel app ##################################

# add_executable(checkTilted2DDPMModel checkTilted2DDPMModel.cxx)
# target_link_libraries(checkTilted2DDPMModel $ENV{ROOT_LIBRARIES} LmdFit)

# ########################## create checkMCInfoTrackMatching app ##################################
# add_executable(checkMCInfoTrackMatching checkMCInfoTrackMatching.cxx)
# target_link_libraries(checkMCInfoTrackMatching ${LINK_LIBS})

# ########################## create makeTalkPlots app ##################################

# add_executable(makeTalkPlots makeTalkPlots.cxx)
# target_link_libraries(makeTalkPlots LmdFit)

# ########################## create DPMModel plotter app ##################################

# add_executable(plotDPMModelParts plotDPMModelParts.cxx)
# target_link_libraries(plotDPMModelParts LmdFit)

# ########################## create DPMModel2D plotter app ##################################
