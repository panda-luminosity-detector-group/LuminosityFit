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
set(LINK_LIBS MathMore PndData LmdTool)

file(GLOB SRCS ./*.cxx)

add_library(LmdFitData SHARED ${SRCS})

target_link_libraries(LmdFitData ${LINK_LIBS})