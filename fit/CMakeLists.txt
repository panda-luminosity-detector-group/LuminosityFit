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

set(ROOT_DICT_HEADERS
fit/ModelFitResult.h
fit/PndLmdFitOptions.h
fit/PndLmdFitStorage.h
LumiFitStructs.h
)

set(LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)

message(STATUS "Generating root dictionary for fit objects...")
execute_process(COMMAND rootcint -f LmdFitDict.cxx -c -I${CMAKE_SOURCE_DIR}/model_framework -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF})
message(STATUS "done!")

set(SRCS
PndLmdFitOptions.cxx
PndLmdLumiFitResult.cxx
PndLmdFitStorage.cxx
${CMAKE_BINARY_DIR}/LmdFitDict.cxx
)

add_library(LmdFit SHARED ${SRCS})

target_link_libraries(LmdFit INTERFACE Model)
