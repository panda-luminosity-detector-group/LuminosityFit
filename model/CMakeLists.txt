set(INCLUDE_DIRECTORIES
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
set(LINK_LIBS Model boost_filesystem boost_regex)

file(GLOB_RECURSE SRCS ./*.cxx)

add_library(LmdModel SHARED ${SRCS})

target_link_libraries(LmdModel ${LINK_LIBS})