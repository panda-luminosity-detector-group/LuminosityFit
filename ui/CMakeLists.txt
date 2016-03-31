set(INCLUDE_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/include
  $ENV{ROOT_INCLUDE_DIR}
  $ENV{BOOST_ROOT}/include
  $ENV{VALGRIND_ROOT}/include/valgrind
)
include_directories(${INCLUDE_DIRECTORIES})


set(LINK_DIRECTORIES
  $ENV{MODEL_FRAMEWORK}/lib
  $ENV{ROOT_LIBRARY_DIR}
  $ENV{BOOST_ROOT}/lib
  $ENV{VALGRIND_ROOT}/lib/valgrind
)

link_directories(${LINK_DIRECTORIES})
set(LINK_LIBS 
LmdFit
LmdFitData
LmdModel
)

set(SRCS
PndLmdDataFacade.cxx
PndLmdFitFacade.cxx
PndLmdPlotter.cxx
PndLmdRuntimeConfiguration.cxx
)

add_library(LmdUI SHARED ${SRCS})

target_link_libraries(LmdUI LINK_PRIVATE ${LINK_LIBS})