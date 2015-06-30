cmake_minimum_required(VERSION 2.6)

project(LmdFit)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")

include_directories(./)

set(ROOT_DICT_HEADERS
fit/ModelFitResult.h
fit/EstimatorOptions.h
fit/PndLmdFitOptions.h 
data/PndLmdFitStorage.h
data/PndLmdFitDataBundle.h
data/PndLmdAbstractData.h
data/PndLmdHistogramData.h
data/PndLmdAngularData.h
data/PndLmdAcceptance.h
LumiFitStructs.h
)

set(LINKDEF LinkDef.h)

execute_process(COMMAND rootcint -f LmdFitDict.cxx -c -I$ENV{MODEL_FRAMEWORK}/include -I${CMAKE_SOURCE_DIR} ${ROOT_DICT_HEADERS} ${LINKDEF})


add_subdirectory(data)
add_subdirectory(fit)
add_subdirectory(model)

add_subdirectory(apps)