#--------------------------------------------------------------------------
# File and Version Information:
#      $Id: standard_FORTRAN.mk,v 1.4 2003/09/11 15:23:06 sokolov Exp $
#
# Description:
# 	Standard standalone makefile for FORTRAN written package 
#	for PANDA software
#
# Environment:
#      Software developed for the PANDA Detector at GSI, Darmstadt
#
# Author List:
#      Sergey Ganzhur                Original Author
#
# Copyright Information:
#      Copyright (C) 2002           Ruhr Universitaet Bochum
#
#------------------------------------------------------------------------

PROGRAM=DPMGen
BINDIR=bin
MAIN=main
PACKAGE=DpmEvtGen

include ./common.mk


LIBA    = $(LIBDIR)/lib$(PACKAGE).a
LIBSO   = $(SLIBDIR)/lib$(PACKAGE).so

# Lists of objects to include in library:


SRCS := $(filter-out $(addsuffix .f, $(MAIN) ), $(wildcard *.f))   

ifdef SRC
        OBJS := $(patsubst %.f,$(OBJDIR)/%.o,$(SRC))
else
        OBJS := $(patsubst %.f,$(OBJDIR)/%.o,$(SRCS))
endif

all : 	lib bin

lib:   	$(LIBA) $(LIBSO)
	@echo ---------------------------------------
	@echo End of $(PACKAGE) package building ...
	@echo ---------------------------------------


shlib: 	$(LIBSO)

bin:	$(PROGRAM)
 
# Static library:
$(LIBA) : $(OBJS)  
	@echo Building  $@ library ...
	rm -f $@
	@if [ ! -d $(LIBDIR) ] ; then mkdir -p $(LIBDIR)  ;fi
ifdef PNDVERBOSE
	$(AR) $(ARFLAGS)  $@ $^
else
	@$(AR) $(ARFLAGS)  $@ $^
endif

# Shared library:
$(LIBSO) : $(OBJS)  
	@echo Building  $@ library ...
	@if [ ! -d $(SLIBDIR) ] ; then mkdir -p $(SLIBDIR)  ;fi
ifdef PNDVERBOSE
	$(LD) -g $(SOFLAGS) $(GLIBS) -o $@ $^
else
	@$(LD) -g $(SOFLAGS) $(GLIBS) -o $@ $^
endif

#
# Complete cleanup:
#

clean : clean_libs
	@echo Removing objs for $(PACKAGE) package ...
	@rm  -f $(OBJDIR)/*.o  
	@if [ -f $(PACKAGE)_LinkDef.hh ] ; then \
	echo Removing  dictionary $(PACKAGE)Cint ... ; \
	rm -f $(PACKAGE)Cint.* ; fi
	@if [ -f $(BINDIR)/$(PROGRAM) ] ; then \
	echo Removing  $(PROGRAM) ... ; \
	rm -f $(BINDIR)/$(PROGRAM) ; fi


clean_libs:
	@echo -----------------------------------
	@echo Cleaning up $(PACKAGE) package ...
	@echo -----------------------------------
	@if [  -f $(LIBA) ] ; then \
	echo Removing library $(LIBA) ... ; \
	rm -f $(LIBA) ; fi
	@if [ -f $(LIBSO) ] ; then \
	echo Removing library $(LIBSO) ... ; \
	rm -f $(LIBSO) ; fi



#
# include binmake: 
#
include ./binmake_FORTRAN.mk











