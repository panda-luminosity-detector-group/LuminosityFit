# $Id: arch_spec_GEANT4.mk,v 1.4 2004/04/19 09:30:01 olaf Exp $
# ----------------------------------------------------------
# Script defining rules and paths for making binaries.
#    First implementation: Gabriele Cosmo, 25/06/1998.
# ----------------------------------------------------------

# Define variable checking for existence of centrally installed headers,
# and global compound libraries installed (in future, the mechanism will
# be extended to the granular libraries as well. The liblist application
# must be re-engineered for that).
# If check will be true, avoid explicit definition of paths to INCFLAGS.
#


CPPFLAGS += $(INCFLAGS)

ifdef LDLIBS
  USER_DEFINED_LDLIBS := 1
endif

# Because of the script for granular libraries which replaces part of LDLIBS
# and because of the way user defined LDLIBS was augmented historically I
# have split LDLIBS into 4 parts...
#
ifndef USER_DEFINED_LDLIBS

# LDLIBS1 contains the very high level libraries...
#

LDLIBS1 += $(EXTRALIBS)

# VISLIBS and UILIBS are now handled by the granular library script...
#
ifdef GLOBALLIBS
  LDLIBS1 += $(VISLIBS) $(UILIBS)
endif

#
ifdef USER_DEFINED_LDLIBS
  LDLIBS_PREFINAL := $(LDLIBS)
else
# Again, do not use := or +=.  See note on LDLIBS2 above.
  LDLIBS_PREFINAL = $(LDLIBS1) $(LDLIBS2) $(LDLIBS2EXTRA) $(LDLIBS3)
endif

ifeq ($(G4SYSTEM),WIN32-VC)
  WIN32TMP := $(patsubst -L%,$(LIB_PATH)%,$(LDFLAGS))
  LDFLAGS  = $(patsubst /,$(PATH_DEL),$(WIN32TMP))
  LDLIBS = $(patsubst -l%,lib%.a,$(LDLIBS_PREFINAL))
else
  LDLIBS = $(LDLIBS_PREFINAL)
endif
endif


