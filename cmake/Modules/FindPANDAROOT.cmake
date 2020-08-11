# This module find the PandaROOT installation and sets up appropriate cmake infos
# It defines:
# PANDAROOT_FOUND             If PandaROOT is found
# PndData library
# LmdTool library
#

if(EXISTS "$ENV{FAIRLIBDIR}/libPndData.so" 
   AND EXISTS "$ENV{VMCWORKDIR}/detectors/lmd/LmdQA/PndLmdTrackQ.h" 
   AND EXISTS $ENV{FAIRROOTPATH}/include)
  SET(PANDAROOT_FOUND TRUE)

  add_library(PndData SHARED IMPORTED)
  set(PandaIncludeDirs
    $ENV{VMCWORKDIR}/pnddata
    $ENV{VMCWORKDIR}/pnddata/SdsData
    $ENV{VMCWORKDIR}/pnddata/TrackData
  )
	set_target_properties(PndData PROPERTIES
    IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libPndData.so"
    INTERFACE_INCLUDE_DIRECTORIES "${PandaIncludeDirs}"
  )
  add_library(LmdTool SHARED IMPORTED)
  set(PandaIncludeDirs
    $ENV{VMCWORKDIR}/detectors/lmd/LmdQA
    $ENV{FAIRROOTPATH}/include
  )
	set_target_properties(LmdTool PROPERTIES
    IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libLmdTool.so"
    INTERFACE_INCLUDE_DIRECTORIES "${PandaIncludeDirs}"
  )
else()
  SET(PANDAROOT_FOUND FALSE)
endif()