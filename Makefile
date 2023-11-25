
include config.mk

.PHONY: all, run, debug, release

all:
	$(MAKE) -C src debug

run: 
	$(PRGNAME) "$(IN_FILE)"

clean:
	$(MAKE) -C src clean

release:
	$(MAKE) -C src release

install: release
	mkdir -p $(DST)/bin
	cp $(PRGNAME) bin/dml $(DST)/bin
	chmod 755 $(DST)/bin/$(PRGNAME)
	chmod 755 $(DST)/bin/dml


