set(ROOT_DICT_HEADERS
data/PndLmdFitDataBundle.h
data/PndLmdAbstractData.h
data/PndLmdHistogramData.h
data/PndLmdAngularData.h
data/PndLmdAcceptance.h
data/PndLmdMapData.h
LumiFitStructs.h
)

set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/model_framework ${CMAKE_SOURCE_DIR})
ROOT_GENERATE_DICTIONARY(LmdFitDataDict ${ROOT_DICT_HEADERS} LINKDEF LinkDef.h)

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
)

add_library(LmdFitData SHARED ${SRCS} LmdFitDataDict.cxx)

add_library(PndData SHARED IMPORTED)
set_target_properties(PndData PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libPndData.so")
add_library(LmdTool SHARED IMPORTED)
set_target_properties(LmdTool PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libLmdTool.so")

target_include_directories(LmdFitData
	PUBLIC ${ROOT_INCLUDE_DIR}
	PRIVATE $ENV{VMCWORKDIR}/detectors/lmd/LmdQA
		$ENV{VMCWORKDIR}/pnddata
		$ENV{VMCWORKDIR}/pnddata/SdsData
		$ENV{VMCWORKDIR}/pnddata/TrackData
		$ENV{FAIRROOTPATH}/include
)
target_link_libraries(LmdFitData
	PUBLIC LmdFit ROOT::Core ROOT::RIO
	PRIVATE PndData LmdTool 
)


add_custom_command(TARGET LmdFitData
		   POST_BUILD
		   COMMAND cp LmdFitDataDict.cxx LmdFitDataDict-copy.cxx
		   COMMAND rootcling -f LmdFitDataDict-copy.cxx -rml libLmdFitData.so -rmf libLmdFitData.rootmap -I${CMAKE_SOURCE_DIR}/model_framework -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h
 		   COMMAND rm LmdFitDataDict-copy.cxx LmdFitDataDict-copy_rdict.pcm
 		   COMMAND cp libLmdFitData.rootmap ${CMAKE_BINARY_DIR}/lib/.
 		   COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/LmdFitDataDict_rdict.pcm
                ${CMAKE_BINARY_DIR}/lib/.
 		   COMMENT "Generating root map..."
)
