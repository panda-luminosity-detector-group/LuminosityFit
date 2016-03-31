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
set(LINK_LIBS 
Model
boost_filesystem
boost_regex
)

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

target_link_libraries(LmdModel LINK_PRIVATE ${LINK_LIBS})