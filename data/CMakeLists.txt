set(INCLUDE_DIRECTORIES
  ${ROOT_INCLUDE_DIR}
  $ENV{VMCWORKDIR}/detectors/lmd/LmdQA
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

message(STATUS "Generating root dictionary for data objects...")
execute_process(COMMAND rootcling -f LmdFitDataDict.cxx -I${CMAKE_SOURCE_DIR}/model_framework -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF})
message(STATUS "done!")

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

target_link_libraries(LmdFitData 
	INTERFACE MathMore LmdFit
	PRIVATE PndData LmdTool)

add_custom_command(TARGET LmdFitData 
		   POST_BUILD 
		   COMMAND rootcling -f LmdFitDataDict.cxx -rml ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libLmdFitData.so -rmf ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libLmdFitData.rootmap -I${CMAKE_SOURCE_DIR}/model_framework -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF}
 		   COMMENT "Generating root map..."
                
		  )
