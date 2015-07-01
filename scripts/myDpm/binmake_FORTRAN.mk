#--------------------------------------------------------------------------
# File and Version Information:
#      $Id: binmake_FORTRAN.mk,v 1.5 2003/09/12 09:28:18 sokolov Exp $
#
# Description:
# 	Standard binary  makefile for FORTRAN written package 
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

$(PROGRAM) : $(OBJDIR)/$(addsuffix .o, $(MAIN) ) $(OBJS)
	@echo Linking binary $@ ...
	@if [ ! -d $(BINDIR) ] ; then mkdir -p $(BINDIR)  ;fi
ifdef PNDVERBOSE
 ifeq ($(PROGRAM), DPMGen)
	$(LD) -o $(BINDIR)/$@ $< $(DPMLIB) $(LDFLAGS) $(LDLIBS)
 endif
else 
 ifeq ($(PROGRAM),DPMGen)
	@$(LD) -o $(BINDIR)/$@ $< $(DPMLIB) $(LDFLAGS) $(LDLIBS)
 endif
endif

