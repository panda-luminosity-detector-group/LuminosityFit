find_package(GSL REQUIRED)

file(GLOB_RECURSE SRCS ./*.cxx)

add_library(Model SHARED ${SRCS})

target_include_directories(Model
	PUBLIC ${Boost_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS}
	PRIVATE ${ROOT_INCLUDE_DIR} 
)
target_link_libraries(Model
	PRIVATE Boost::thread Boost::filesystem Boost::system ${ROOT_LIBRARIES} ${GSL_LIBRARIES}
)

install(TARGETS Model LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib)

get_filename_component(BUILD_DIR ${CMAKE_BINARY_DIR} NAME)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/ 
        DESTINATION ${CMAKE_BINARY_DIR}/include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "${BUILD_DIR}" EXCLUDE
        PATTERN ".git" EXCLUDE
        )
