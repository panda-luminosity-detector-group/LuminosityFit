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

set(SRCS
PndLmdDataFacade.cxx
PndLmdFitFacade.cxx
PndLmdPlotter.cxx
PndLmdRuntimeConfiguration.cxx
)

add_library(LmdUI SHARED ${SRCS})

target_link_libraries(LmdUI 
	PUBLIC LmdFitData LmdFit Physics RIO Matrix
	PRIVATE boost_regex
	INTERFACE LmdModel
)
