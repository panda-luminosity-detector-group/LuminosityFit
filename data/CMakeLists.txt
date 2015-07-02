set(ROOT_DICT_HEADERS
fit/ModelFitResult.h
fit/EstimatorOptions.h
fit/PndLmdFitOptions.h
fit/PndLmdFitStorage.h
data/PndLmdFitDataBundle.h
data/PndLmdAbstractData.h
data/PndLmdHistogramData.h
data/PndLmdAngularData.h
data/PndLmdAcceptance.h
LumiFitStructs.h
)

set(LINKDEF LinkDef.h)

execute_process(COMMAND rootcint -f LmdFitDataDict.cxx -c -I$ENV{MODEL_FRAMEWORK}/include -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF})

set(INCLUDE_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/include
  $ENV{ROOT_INCLUDE_DIR}
  $ENV{VMCWORKDIR}/lmd/LmdQA
  $ENV{VMCWORKDIR}/pnddata
  $ENV{VMCWORKDIR}/pnddata/SdsData
  $ENV{VMCWORKDIR}/pnddata/TrackData
  $ENV{VMCWORKDIR}/trackbase
  $ENV{VMCWORKDIR}/base/event
  $ENV{BOOST_ROOT}/include
)
include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  $ENV{ROOT_LIBRARY_DIR}
  $ENV{FAIRLIBDIR}
  $ENV{BOOST_ROOT}/lib
)
link_directories(${LINK_DIRECTORIES})

set(LINK_LIBS
MathMore
PndData
LmdTool
)

set(SRCS
PndLmdAbstractData.cxx
PndLmdAcceptance.cxx
PndLmdAngularData.cxx
PndLmdCombinedDataReader.cxx
PndLmdDataReader.cxx
PndLmdFitDataBundle.cxx
PndLmdHistogramData.cxx
PndLmdSeperateDataReader.cxx
${CMAKE_BINARY_DIR}/LmdFitDataDict.cxx
)

add_library(LmdFitData SHARED ${SRCS})

target_link_libraries(LmdFitData LINK_PRIVATE ${LINK_LIBS})