set(INCLUDE_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/include
  $ENV{ROOT_INCLUDE_DIR}
  $ENV{BOOST_ROOT}/include
)
include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/lib
  $ENV{ROOT_LIBRARY_DIR}
  $ENV{BOOST_ROOT}/lib
)
link_directories(${LINK_DIRECTORIES})



########################### create phiSlicing app ##################################

#add_executable(phiSliceData phiSliceData.cxx)
#target_link_libraries(phiSliceData Lmd LmdTool MathMore Eve)

########################### create lumiFitData app ##################################

add_executable(createLmdFitData createLmdFitData.cxx)
target_link_libraries(createLmdFitData LmdFitData boost_regex)

########################### create mergeLmdData app ##################################

add_executable(mergeLmdData mergeLmdData.cxx)
target_link_libraries(mergeLmdData LmdFit boost_regex)

########################### create lumiFit app ##################################

add_executable(runLmdFit runLmdFit.cxx)
target_link_libraries(runLmdFit LmdFit)

########################### create determineBeamOffset app ##################################

add_executable(determineBeamOffset determineBeamOffset.cxx)
target_link_libraries(determineBeamOffset LmdFit)

########################### create lumiFitSystematic app ##################################

add_executable(determineLumiFitSystematics determineLumiFitSystematics.cxx)
target_link_libraries(determineLumiFitSystematics LmdFit boost_regex)

########################### create acceptanceComparison app ##################################

add_executable(compareAcceptances compareAcceptances.cxx)
target_link_libraries(compareAcceptances LmdFit)

########################### create fitRawDPMElasticData app ##################################

#add_executable(fitRawDPMElasticData fitRawDPMElasticData.cxx)
#target_link_libraries(fitRawDPMElasticData LmdFit)

########################### create resolutionModelFitTesting1D app ##################################

#add_executable(resolutionModelFitTesting1D resolutionModelFitTesting1D.cxx)
#target_link_libraries(resolutionModelFitTesting1D LmdFit)

########################### create checkFramework app ##################################

add_executable(checkFramework checkFramework.cxx)
target_link_libraries(checkFramework $ENV{ROOT_LIBRARIES} Model)

########################### create checkTilted2DDPMModel app ##################################

#add_executable(checkTilted2DDPMModel checkTilted2DDPMModel.cxx)
#target_link_libraries(checkTilted2DDPMModel $ENV{ROOT_LIBRARIES} LmdFit)

########################### create checkMCInfoTrackMatching app ##################################
add_executable(checkMCInfoTrackMatching checkMCInfoTrackMatching.cxx)
target_link_libraries(checkMCInfoTrackMatching ${LINK_LIBS})

########################### create elastic cross section integrator app ##################################

add_executable(calculateElasticCrossSection calculateElasticCrossSection.cxx)
target_link_libraries(calculateElasticCrossSection LmdModel LmdFitData)

########################### create makeTalkPlots app ##################################

#add_executable(makeTalkPlots makeTalkPlots.cxx)
#target_link_libraries(makeTalkPlots LmdFit)

########################### create BeamOffsetResult plotter app ##################################

add_executable(plotIPDistribution plotIPDistribution.cxx)
target_link_libraries(plotIPDistribution LmdFit)

########################### create LumiFit plotter app ##################################

add_executable(plotLumiFitResults plotLumiFitResults.cxx)
target_link_libraries(plotLumiFitResults LmdFit)

########################### create lumi resolution plotter app ##################################

#add_executable(plotResolutionParametrization plotResolutionParametrization.cxx)
#target_link_libraries(plotResolutionParametrization LmdFit)

########################### create DPMModel plotter app ##################################

#add_executable(plotDPMModelParts plotDPMModelParts.cxx)
#target_link_libraries(plotDPMModelParts LmdFit)

########################### create DPMModel2D plotter app ##################################

#add_executable(plotShifted2DDPMModel plotShifted2DDPMModel.cxx)
#target_link_libraries(plotShifted2DDPMModel LmdFit)