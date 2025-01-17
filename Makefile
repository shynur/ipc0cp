SHELL = bash
CXX := $(shell echo $${CXX:-g++}) -std=c++$(shell echo $${ISOCPP:-26})
DEBUG != if (: $${NDEBUG:?}) 2> /dev/null; then :; else echo 1; fi
CXXFLAGS := -Wpedantic -Wall -W $(if $(DEBUG),-O0 -ggdb -g3,-g0 -O3 -D'NDEBUG') -Iinclude

LIBS = fmt
LIBARS := $(LIBS:%=lib/archives/lib%.a)
LDFLAGS := -lrt -pthread $(if $(LIBS), -l$(LIBS))

READERS != bash -c "echo reader-{1..`cat include/ipc0cp.hpp | grep 'num_readers =' - | awk -F'= |;' '{printf $$2}'`}"
MSG = MessageLaser

BUILD_INFO := $(if $(DEBUG),beta,rc)-$(shell basename `echo $(CXX) | awk -F' ' '{printf $$1}'`)-C++$(shell echo $${ISOCPP:-26})

# ----------------------------------------------------------

.DEFAULT_GOAL = run
.PHONY: run
run:  bin/writer-$(BUILD_INFO).exe  $(addprefix bin/,$(addsuffix -$(BUILD_INFO).exe,$(READERS)))
	ls -l --almost-all --color=always /dev/shm/
	rm -f /dev/shm/*ipc0cp-?* /dev/shm/*ipcator-?*
	for exe in $^; do  \
	  ( if echo $$exe | grep writer - > /dev/null; then  \
        nice -n 19 ./$$exe;  \
      else  \
	      if ((`id -u` == 0)); then  \
  	      echo "以 \`sudo nice\` 执行 \`$$exe\`";  \
	        nice -n -20 ./$$exe;  \
	      else  \
	        ./$$exe;  \
	      fi;  \
      fi;  \
	    echo  \
	  ) &  \
	done;  \
	wait
	rm -f /dev/shm/*ipc0cp-?* /dev/shm/*ipcator-?*


bin/writer-$(BUILD_INFO).exe:  obj/writer-$(BUILD_INFO).o  $(LIBARS)  |  bin/
	$(CXX) $< -L./lib/archives $(LDFLAGS) -o $@
	-chmod a+x $@

bin/reader-%-$(BUILD_INFO).exe:  obj/reader-%-$(BUILD_INFO).o  $(LIBARS)  |  bin/
	$(CXX) $< -L./lib/archives $(LDFLAGS) -o $@
	-chmod a+x $@


obj/writer-$(BUILD_INFO).o:  src/writer.cpp  include/ipc0cp.hpp  include/$(MSG).fbs.hpp  |  obj/
	$(CXX) -c $(CXXFLAGS) $< -o $@
	-chmod a+x $@

obj/reader-%-$(BUILD_INFO).o:  src/reader-%.cpp  include/ipc0cp.hpp  include/$(MSG).fbs.hpp  |  obj/
	$(CXX) -c $(CXXFLAGS) $< -o $@
	-chmod a+x $@


src/reader-%.cpp:  src/reader.cpp
	cp -f $< $@
	-chmod a+rw $@


include/%.fbs.hpp:  protos/%.fbs
	cd protos;  \
	flatc `#--gen-mutable` --reflect-names --cpp $(MSG).fbs
	mv protos/$(MSG)_generated.h include/$(MSG).fbs.hpp
.NOTINTERMEDIATE:  protos/$(MSG).fbs
protos/%.fbs:  protos/%.proto
	cd protos;  \
	flatc --proto $(MSG).proto
	echo $$'\nroot_type $(MSG);' >> $@

lib/archives/libfmt.a:
	NDEBUG=$(if $(DEBUG),,1) make --makefile=lib/ipcator/Makefile $@

%/:
	mkdir -p $@
	-chmod -R a+rwx $@

# ----------------------------------------------------------

.PHONY: clean
clean:
	rm -rf  bin/ obj/
	rm -f   include/?*.fbs.hpp
	rm -rf  lib/?*-build/
	rm -rf  lib/archives/
	rm -f   protos/$(MSG).fbs

.PHONY: git
git:
	git commit -av
	git push

.PHONY: proto
proto: include/$(MSG).fbs.hpp

.PHONY: print-vars
print-vars:
	@echo READERS = $(READERS)
	@echo MSG = $(MSG)
	@echo CXX = $(CXX)
	@echo DEBUG = $(DEBUG)
	@echo CXXFLAGS = $(CXXFLAGS)
	@echo LIBS = $(LIBS)
	@echo LIBDIRS = $(LIBDIRS)
	@echo LIBFLAGS = $(LIBFLAGS)
	@echo LDFLAGS = $(LDFLAGS)
	@echo BUILD_INFO = $(BUILD_INFO)
