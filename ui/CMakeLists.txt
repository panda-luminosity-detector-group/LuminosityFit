add_library(LmdRuntimeConfig SHARED PndLmdRuntimeConfiguration.cxx)
target_include_directories(LmdRuntimeConfig
	PRIVATE ${ROOT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(LmdRuntimeConfig 
	PUBLIC Boost::filesystem
	PRIVATE ${ROOT_LIBRARIES} Boost::regex
)

set(SRCS
	PndLmdFitFacade.cxx
	PndLmdPlotter.cxx
)
if (PANDAROOT_FOUND)
	list(APPEND SRCS PndLmdDataFacade.cxx)
endif()

add_library(LmdUI SHARED ${SRCS})

target_include_directories(LmdUI
	PRIVATE ${ROOT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(LmdUI 
	PUBLIC LmdFitData LmdFit LmdRuntimeConfig Boost::filesystem
	PRIVATE ${ROOT_LIBRARIES} Boost::regex
	INTERFACE LmdModel
)

if (PANDAROOT_FOUND)
	target_link_libraries(LmdUI 
		PUBLIC LmdFitPandaDataIO
	)
endif()