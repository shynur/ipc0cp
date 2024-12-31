SHELL = bash
CXX = $(shell echo $${CXX:-g++}) -std=c++$(shell echo $${ISOCPP:-26})
CXXFLAGS = -Wpedantic -Wall -W -O0 -ggdb -g3 -Iinclude
LIBS = fmt
LIBDIRS = $(if $(LIBS),./lib/$(LIBS)-build/)
LIBFLAGS = $(if $(LIBS), -L$(LIBDIRS))
LDFLAGS = -lrt -pthread $(if $(LIBS), -l$(LIBS))

READERS = reader-1
PROTO = laser

BUILD_INFO = $(shell basename `echo $(CXX) | awk -F' ' '{printf $$1}'`)-C++$(shell echo $${ISOCPP:-26})

# ----------------------------------------------------------

.PHONY: run
run:  bin/writer-$(BUILD_INFO).exe  bin/$(READERS)-$(BUILD_INFO).exe
	ls -al /dev/shm/
	for exe in $^; do  \
	    (((`id -u`)) && ./$$exe || nice -n -20 ./$$exe; echo) &  \
	done;  \
	wait
	rm -f /dev/shm/*ipc0cp-?* /dev/shm/*ipcator-?*

bin/writer-$(BUILD_INFO).exe:  obj/writer-$(BUILD_INFO).o  $(LIBDIRS)
	mkdir -p bin
	$(CXX) $< $(LIBFLAGS) $(LDFLAGS) -o $@

bin/reader-%-$(BUILD_INFO).exe:  obj/reader-%-$(BUILD_INFO).o  $(LIBDIRS)
	mkdir -p bin
	$(CXX) $< $(LIBFLAGS) $(LDFLAGS) -o $@

obj/writer-$(BUILD_INFO).o:  src/writer.cpp  include/ipc0cp.hpp  include/$(PROTO).fbs.hpp
	mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $< -o $@

obj/reader-%-$(BUILD_INFO).o:  src/reader-%.cpp  include/ipc0cp.hpp  include/$(PROTO).fbs.hpp
	mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $< -o $@

include/%.fbs.hpp:  protos/%.fbs
	make proto

lib/fmt-build/:  lib/fmt-build/libfmt.a
lib/fmt-build/libfmt.a:
	make --makefile=lib/ipcator/Makefile $@

# ----------------------------------------------------------

.PHONY: clean
clean:
	rm -rf  bin/ obj/
	rm -f   include/?*.fbs.hpp
	rm -rf  lib/?*-build/

.PHONY: git
git:
	git commit -av
	git push

.PHONY: proto
proto:
	flatc `#--gen-mutable` --reflect-names --cpp protos/$(PROTO).fbs
	(($$?==0))  &&  mv $(PROTO)_generated.h include/$(PROTO).fbs.hpp

.PHONY: print-vars
print-vars:
	@echo CXX = $(CXX)
	@echo DEBUG = $(DEBUG)
	@echo CXXFLAGS = $(CXXFLAGS)
	@echo LIBS = $(LIBS)
	@echo LIBDIRS = $(LIBDIRS)
	@echo LIBFLAGS = $(LIBFLAGS)
	@echo LDFLAGS = $(LDFLAGS)
	@echo BUILD_INFO = $(BUILD_INFO)
