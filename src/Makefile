
include ../config.mk

 OBJ = base64.o dml.o execute.o file_parser.o main.o \
				red_black_tree.o shell_command.o 

.PHONY: all, run, debug, release

all: debug

run: 
	../$(PRGNAME) "$(IN_FILE)"

debug: $(OBJ)
	$(CC) -o ../$(PRGNAME) $(LDFLAGS) $(OBJ)

# release: $(OBJ)
# 	$(CC) -o ../$(PRGNAME) $(RFLAGS) $(OBJ)

%.o: %.cpp
	$(CC) -c $(CFLAGS) -o $@ $<
	
clean:
	rm -f _dml_composer $(OBJ)

install: all
	mkdir -p $(DST)/bin
	cp $(PRGNAME) bin/dml $(DST)/bin
	chmod 755 $(DST)/bin/$(PRGNAME)
	chmod 755 $(DST)/bin/dml