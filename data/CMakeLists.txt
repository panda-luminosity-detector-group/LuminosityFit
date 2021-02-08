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
	PndLmdFitDataBundle.cxx
	PndLmdHistogramData.cxx
)

add_library(LmdFitData SHARED ${SRCS} LmdFitDataDict.cxx)

target_include_directories(LmdFitData
	PUBLIC ${ROOT_INCLUDE_DIR}
)
target_link_libraries(LmdFitData
	PUBLIC LmdFit ROOT::Core ROOT::RIO ROOT::Tree
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

add_library(LmdFitDataReader SHARED PndLmdDataReader.cxx)

target_include_directories(LmdFitDataReader
	PUBLIC ${ROOT_INCLUDE_DIR}
)
target_link_libraries(LmdFitDataReader
	PUBLIC LmdFitData
)




if (PANDAROOT_FOUND)
	set(SRCS
		PndLmdCombinedDataReader.cxx
		PndLmdSeperateDataReader.cxx
	)
	add_library(LmdFitPandaDataIO SHARED ${SRCS})
	target_link_libraries(LmdFitPandaDataIO
		PUBLIC ROOT::RIO LmdFitDataReader
		PRIVATE PndData LmdTool 
	)
else()
	message(STATUS "No PANDAROOT environment found! Not building PANDA Data reader library.")
endif()

if (KOALASOFT_FOUND)
	set(SRCS
		KoaSeperateDataReader.cxx
	)
	add_library(LmdFitKoaDataIO SHARED ${SRCS})
	target_link_libraries(LmdFitKoaDataIO
		PUBLIC ROOT::RIO LmdFitDataReader Boost::serialization
		 KoaData KoaTools SdsData 
	)
	
else()
	message(STATUS "No KOALASOFT environment found! Not building KOALA Data reader library.")
endif()


