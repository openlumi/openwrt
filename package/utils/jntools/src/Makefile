# ------------------------------------------------------------------
# JN Flasher Makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

SUBDIRS = jennicProgrammer
# jennicDetector cbusFtdi scripts

.PHONY: subdirs $(SUBDIRS)

DOIT = $(MAKECMDGOALS)

all: $(SUBDIRS)

build: $(SUBDIRS)

clean: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) $(DOIT) -C $@


