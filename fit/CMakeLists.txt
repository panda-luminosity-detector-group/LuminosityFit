set(ROOT_DICT_HEADERS
fit/ModelFitResult.h
fit/PndLmdFitOptions.h
fit/PndLmdFitStorage.h
LumiFitStructs.h
)

set(INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/model_framework ${CMAKE_SOURCE_DIR})
ROOT_GENERATE_DICTIONARY(LmdFitDict ${ROOT_DICT_HEADERS} LINKDEF LinkDef.h)

set(SRCS
PndLmdFitOptions.cxx
PndLmdLumiFitResult.cxx
PndLmdFitStorage.cxx
)

add_library(LmdFit SHARED ${SRCS} LmdFitDict.cxx)

target_include_directories(LmdFit 
	PUBLIC ${ROOT_INCLUDE_DIR})
target_link_libraries(LmdFit
	INTERFACE Model ROOT::Core)

add_custom_command(
        TARGET LmdFit POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/*.pcm
                ${CMAKE_BINARY_DIR}/lib/.
        COMMENT "copying LmdFit root pcm file..."
)