SHELL = bash
CXX = $(shell echo $${CXX:-g++}) -std=c++$(shell echo $${ISOCPP:-26})
DEBUG = $(shell if (: $${NDEBUG:?}) 2> /dev/null; then :; else echo 1; fi)
CXXFLAGS = -Wpedantic -Wall -W $(if $(DEBUG),-O0 -ggdb -g3,-g0 -O3 -D'NDEBUG') -Iinclude
LIBS = fmt
LIBDIRS = $(if $(LIBS),./lib/$(LIBS)-build/)
LIBFLAGS = $(if $(LIBS), -L$(LIBDIRS))
LDFLAGS = -lrt -pthread $(if $(LIBS), -l$(LIBS))

READERS = reader-1
PROTO = laser

BUILD_INFO = $(if $(DEBUG),beta,rc)-$(shell basename `echo $(CXX) | awk -F' ' '{printf $$1}'`)-C++$(shell echo $${ISOCPP:-26})

# ----------------------------------------------------------

.PHONY: run
run:  bin/writer-$(BUILD_INFO).exe  bin/$(READERS)-$(BUILD_INFO).exe
	ls -l --almost-all --color=always /dev/shm/
	rm -f /dev/shm/*ipc0cp-?* /dev/shm/*ipcator-?*
	for exe in $^; do  \
	  (  \
	    if ((`id -u` == 0)); then  \
	      echo '以 `sudo nice` 执行';  \
	      nice -n -20 ./$$exe;  \
	    else  \
	      ./$$exe;  \
	    fi;  \
	    echo  \
	  ) &  \
	done;  \
	wait
	rm -f /dev/shm/*ipc0cp-?* /dev/shm/*ipcator-?*


bin/writer-$(BUILD_INFO).exe:  obj/writer-$(BUILD_INFO).o  $(LIBDIRS)
	mkdir -p bin
	chmod a+rwx bin  ||  true
	$(CXX) $< $(LIBFLAGS) $(LDFLAGS) -o $@
	chmod a+x $@  ||  true

bin/reader-%-$(BUILD_INFO).exe:  obj/reader-%-$(BUILD_INFO).o  $(LIBDIRS)
	mkdir -p bin
	chmod a+rwx bin  ||  true
	$(CXX) $< $(LIBFLAGS) $(LDFLAGS) -o $@
	chmod a+x $@  ||  true

obj/writer-$(BUILD_INFO).o:  src/writer.cpp  include/ipc0cp.hpp  include/$(PROTO).fbs.hpp
	mkdir -p obj
	chmod a+rwx obj  ||  true
	$(CXX) -c $(CXXFLAGS) $< -o $@
	chmod a+x $@  ||  true

obj/reader-%-$(BUILD_INFO).o:  src/reader-%.cpp  include/ipc0cp.hpp  include/$(PROTO).fbs.hpp
	mkdir -p obj
	chmod a+rwx obj  ||  true
	$(CXX) -c $(CXXFLAGS) $< -o $@
	chmod a+x $@  ||  true

include/%.fbs.hpp:  protos/%.fbs
	make proto

lib/fmt-build/:  lib/fmt-build/libfmt.a
	chmod -R a+rwx $@  ||  true
lib/fmt-build/libfmt.a:
	NDEBUG=$(if $(DEBUG),,1) make --makefile=lib/ipcator/Makefile $@

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
