set(INCLUDE_DIRECTORIES
  ./
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
)

set(ROOT_DICT_HEADERS
fit/ModelFitResult.h
fit/PndLmdFitOptions.h
fit/PndLmdFitStorage.h
LumiFitStructs.h
)

set(LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/LinkDef.h)

execute_process(COMMAND rootcint -f LmdFitDict.cxx -c -I$ENV{MODEL_FRAMEWORK}/include -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF})

set(SRCS
PndLmdFitOptions.cxx
PndLmdLumiFitResult.cxx
PndLmdFitStorage.cxx
${CMAKE_BINARY_DIR}/LmdFitDict.cxx
)

add_library(LmdFit SHARED ${SRCS})

target_link_libraries(LmdFit LINK_PRIVATE ${LINK_LIBS})
