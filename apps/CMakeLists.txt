set(INCLUDE_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/include
  $ENV{ROOT_INCLUDE_DIR}
  $ENV{BOOST_ROOT}/include
  $ENV{VALGRIND_ROOT}/include/valgrind
)
include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/lib
  $ENV{ROOT_LIBRARY_DIR}
  $ENV{BOOST_ROOT}/lib
  $ENV{VALGRIND_ROOT}/lib/valgrind
)
link_directories(${LINK_DIRECTORIES})



########################### create elastic cross section integrator app ##################################

add_executable(calculateElasticCrossSection calculateElasticCrossSection.cxx)
target_link_libraries(calculateElasticCrossSection LmdUI)

########################### create checkFramework app ##################################

add_executable(checkFramework checkFramework.cxx)
target_link_libraries(checkFramework $ENV{ROOT_LIBRARIES} Model)

########################### create fitRawDPMElasticData app ##################################

#add_executable(fitRawDPMElasticData fitRawDPMElasticData.cxx)
#target_link_libraries(fitRawDPMElasticData LmdUI)

########################### create checkDivergenceSmearing app ##################################

add_executable(checkDivergenceSmearing checkDivergenceSmearing.cxx)
target_link_libraries(checkDivergenceSmearing $ENV{ROOT_LIBRARIES} LmdUI)

########################### create acceptanceComparison app ##################################

add_executable(compareAcceptances compareAcceptances.cxx)
target_link_libraries(compareAcceptances LmdUI)

add_executable(analyzeAcceptances analyzeAcceptances.cxx)
target_link_libraries(analyzeAcceptances LmdUI)

########################### create lumiFitData app ##################################

add_executable(createLmdFitData createLmdFitData.cxx)
target_link_libraries(createLmdFitData LmdUI)

########################### create determineBeamOffset app ##################################

add_executable(determineBeamOffset determineBeamOffset.cxx)
target_link_libraries(determineBeamOffset LmdUI)

########################### create lumiFitSystematic app ##################################

add_executable(determineLumiFitSystematics determineLumiFitSystematics.cxx)
target_link_libraries(determineLumiFitSystematics LmdUI)

########################### create lumi resolution plotter app ##################################

add_executable(makeResolutionPlots makeResolutionPlots.cxx)
target_link_libraries(makeResolutionPlots LmdUI)

########################### create mergeLmdData app ##################################

add_executable(mergeLmdData mergeLmdData.cxx)
target_link_libraries(mergeLmdData LmdUI)

########################### create BeamOffsetResult plotter app ##################################

add_executable(plotIPDistribution plotIPDistribution.cxx)
target_link_libraries(plotIPDistribution LmdUI)

########################### create LumiFit plotter app ##################################

add_executable(plotLumiFitResults plotLumiFitResults.cxx)
target_link_libraries(plotLumiFitResults LmdUI)

########################### create lumiFit app ##################################

add_executable(runLmdFit runLmdFit.cxx)
target_link_libraries(runLmdFit LmdUI)





########################### create phiSlicing app ##################################

#add_executable(phiSliceData phiSliceData.cxx)
#target_link_libraries(phiSliceData Lmd LmdTool MathMore Eve)

########################### create resolutionModelFitTesting1D app ##################################

#add_executable(resolutionModelFitTesting1D resolutionModelFitTesting1D.cxx)
#target_link_libraries(resolutionModelFitTesting1D LmdFit)

########################### create checkTilted2DDPMModel app ##################################

#add_executable(checkTilted2DDPMModel checkTilted2DDPMModel.cxx)
#target_link_libraries(checkTilted2DDPMModel $ENV{ROOT_LIBRARIES} LmdFit)

########################### create checkMCInfoTrackMatching app ##################################
#add_executable(checkMCInfoTrackMatching checkMCInfoTrackMatching.cxx)
#target_link_libraries(checkMCInfoTrackMatching ${LINK_LIBS})

########################### create makeTalkPlots app ##################################

#add_executable(makeTalkPlots makeTalkPlots.cxx)
#target_link_libraries(makeTalkPlots LmdFit)

########################### create DPMModel plotter app ##################################

#add_executable(plotDPMModelParts plotDPMModelParts.cxx)
#target_link_libraries(plotDPMModelParts LmdFit)

########################### create DPMModel2D plotter app ##################################

add_executable(plot2DDPMModel plot2DDPMModel.cxx)
target_link_libraries(plot2DDPMModel LmdUI)