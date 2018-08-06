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
set(LINK_LIBS Core Hist MathCore Gpad Graf gsl gslcblas boost_thread boost_filesystem boost_system) 


file(GLOB_RECURSE SRCS ./*.cxx)

add_library(Model SHARED ${SRCS})

target_link_libraries(Model ${LINK_LIBS})

install(TARGETS Model LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib)

get_filename_component(BUILD_DIR ${CMAKE_BINARY_DIR} NAME)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/ 
        DESTINATION ${CMAKE_BINARY_DIR}/include
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "${BUILD_DIR}" EXCLUDE
        PATTERN ".git" EXCLUDE
        )
