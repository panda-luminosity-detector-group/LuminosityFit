# This module find the PandaROOT installation and sets up appropriate cmake infos
# It defines:
# KOALASOFT_FOUND             If KOALASOFT is found
# PndData library
# LmdTool library
#

if(EXISTS "$ENV{FAIRLIBDIR}/libKoaData.so" 
   AND EXISTS "$ENV{VMCWORKDIR}/detectors/lmdforward/LmdTrack/KoaComp.h" 
   AND EXISTS $ENV{FAIRROOTPATH}/include)
  SET(KOALASOFT_FOUND TRUE)

  add_library(KoaData SHARED IMPORTED)
  set(PandaIncludeDirs
  $ENV{VMCWORKDIR}/data
  $ENV{VMCWORKDIR}/data/MCData
    $ENV{VMCWORKDIR}/data/TrackData
    $ENV{VMCWORKDIR}/data/SdsData
  )
	set_target_properties(KoaData PROPERTIES
    IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libKoaData.so"
    INTERFACE_INCLUDE_DIRECTORIES "${PandaIncludeDirs}"
  )

  add_library(TrkBase SHARED IMPORTED)
  set(PandaIncludeDirs
  $ENV{FAIRROOTPATH}/include
  )
	set_target_properties(TrkBase PROPERTIES
    IMPORTED_LOCATION "$ENV{FAIRROOTPATH}/lib/libTrkBase.so"
    INTERFACE_INCLUDE_DIRECTORIES "${PandaIncludeDirs}"
  )


  add_library(SdsData SHARED IMPORTED)
  set(PandaIncludeDirs
  $ENV{VMCWORKDIR}/data
    $ENV{VMCWORKDIR}/data/TrackData
    $ENV{VMCWORKDIR}/data/SdsData

  )
	set_target_properties(SdsData PROPERTIES
    IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libSdsData.so"
    INTERFACE_INCLUDE_DIRECTORIES "${PandaIncludeDirs}"
    INTERFACE_LINK_LIBRARIES TrkBase
  )
  
  add_library(KoaTools SHARED IMPORTED)
  set(PandaIncludeDirs
    $ENV{VMCWORKDIR}/detectors/lmdforward/LmdQA
    $ENV{FAIRROOTPATH}/include
  )
	set_target_properties(KoaTools PROPERTIES
    IMPORTED_LOCATION "$ENV{FAIRLIBDIR}/libKoaTools.so"
    INTERFACE_INCLUDE_DIRECTORIES "${PandaIncludeDirs}"
  )
else()
  SET(KOALASOFT_FOUND FALSE)
endif()
