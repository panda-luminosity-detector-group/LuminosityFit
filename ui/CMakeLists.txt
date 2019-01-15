set(SRCS
PndLmdDataFacade.cxx
PndLmdFitFacade.cxx
PndLmdPlotter.cxx
PndLmdRuntimeConfiguration.cxx
)

add_library(LmdUI SHARED ${SRCS})

target_include_directories(LmdUI
	PRIVATE ${ROOT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(LmdUI 
	PUBLIC LmdFitData LmdFit Boost::filesystem
	PRIVATE ${ROOT_LIBRARIES} Boost::regex
	INTERFACE LmdModel
)
