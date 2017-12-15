set(INCLUDE_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/include
  ${ROOT_INCLUDE_DIR}
  $ENV{VMCWORKDIR}/lmd/LmdQA
  $ENV{VMCWORKDIR}/pnddata
  $ENV{VMCWORKDIR}/pnddata/SdsData
  $ENV{VMCWORKDIR}/pnddata/TrackData
  $ENV{FAIRROOTPATH}/include
  $ENV{BOOST_ROOT}/include
)
include_directories(${INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  $ENV{FAIRLIBDIR}
  $ENV{BOOST_ROOT}/lib
)
link_directories(${LINK_DIRECTORIES})

set(LINK_LIBS
MathMore
PndData
LmdTool
LmdFit
)

set(ROOT_DICT_HEADERS
data/PndLmdFitDataBundle.h
data/PndLmdAbstractData.h
data/PndLmdHistogramData.h
data/PndLmdAngularData.h
data/PndLmdAcceptance.h
data/PndLmdMapData.h
LumiFitStructs.h
)

set(LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)

MESSAGE("Generating root dictionary...")
execute_process(COMMAND rootcling -f LmdFitDataDict.cxx -I$ENV{MODEL_FRAMEWORK}/include -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF})
MESSAGE("done!")

set(SRCS
PndLmdAbstractData.cxx
PndLmdAcceptance.cxx
PndLmdAngularData.cxx
PndLmdMapData.cxx
PndLmdCombinedDataReader.cxx
PndLmdDataReader.cxx
PndLmdFitDataBundle.cxx
PndLmdHistogramData.cxx
PndLmdSeperateDataReader.cxx
${CMAKE_BINARY_DIR}/LmdFitDataDict.cxx
)

add_library(LmdFitData SHARED ${SRCS})

target_link_libraries(LmdFitData LINK_PRIVATE ${LINK_LIBS})

add_custom_command(TARGET LmdFitData 
		   POST_BUILD 
		   COMMAND rootcling -f LmdFitDataDict.cxx -rml ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libLmdFitData.so -rmf ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libLmdFitData.rootmap -I$ENV{MODEL_FRAMEWORK}/include -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF}
 		   COMMENT "Generating root map..."
                
		  )
