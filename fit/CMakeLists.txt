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
set(LINK_LIBS Model)

file(GLOB_RECURSE SRCS ./*.cxx)

add_library(LmdFit SHARED ${SRCS})

target_link_libraries(LmdFit ${LINK_LIBS})