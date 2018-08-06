set(INCLUDE_DIRECTORIES
  ${ROOT_INCLUDE_DIR}
  $ENV{BOOST_ROOT}/include
)
include_directories(${INCLUDE_DIRECTORIES})


set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  $ENV{BOOST_ROOT}/lib
)
link_directories(${LINK_DIRECTORIES})


set(SRCS
AsymmetricDoubleGaussianModel1D.cxx
AsymmetricGaussianModel1D.cxx
CachedModel2D.cxx
PndLmdBackgroundModel1D.cxx
PndLmdDPMAngModel1D.cxx
PndLmdDPMAngModel2D.cxx
PndLmdDPMMTModel1D.cxx
PndLmdDPMModelParametrization.cxx
PndLmdDifferentialSmearingConvolutionModel2D.cxx
PndLmdDivergenceSmearingModel2D.cxx
PndLmdE760LikeModelParametrization.cxx
PndLmdE760ModelParametrization.cxx
PndLmdFastDPMAngModel2D.cxx
PndLmdModelFactory.cxx
PndLmdROOTDataModel1D.cxx
PndLmdROOTDataModel2D.cxx
PndLmdSignalBackgroundModel1D.cxx
PndLmdSmearingConvolutionModel2D.cxx
PndLmdSmearingDoubleGaussianModelParametrization1D.cxx
PndLmdSmearingGaussianModelParametrization1D.cxx
PndLmdSmearingGaussianModelParametrization2D.cxx
PndLmdSmearingModel2D.cxx
)

add_library(LmdModel SHARED ${SRCS})

target_link_libraries(LmdModel 
	PUBLIC Model boost_filesystem boost_regex
	PRIVATE Physics EG pthread
	INTERFACE LmdFitData )
