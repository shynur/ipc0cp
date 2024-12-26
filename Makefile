SHELL = bash
CXX = $(shell echo $${CXX:-g++}) -fdiagnostics-color=always -std=c++26
CXXFLAGS = -Wall -W -O0 -ggdb -g3 -Iinclude
LDFLAGS = -lpthread -lrt

READERS = reader-1
PROTO = laser


.PHONY: run
.SILENT: run
run:  bin/writer.exe bin/$(READERS).exe
	rm -f /dev/shm/ipc0cp-*?
	bin/writer.exe &
	for reader in $(READERS); do  \
	    bin/$$reader.exe &        \
	done
	wait

bin/writer.exe:  obj/writer.o
	mkdir -p bin
	$(CXX) $^ $(LDFLAGS) -o $@

bin/$(READERS).exe:  obj/$(READERS).o
	mkdir -p bin
	$(CXX) $^ $(LDFLAGS) -o $@

obj/writer.o:  src/writer.cpp include/ipc0cp.hpp include/$(PROTO).fbs.hpp
	mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $< -o $@

obj/reader-%.o: src/reader-%.cpp include/ipc0cp.hpp include/$(PROTO).fbs.hpp
	mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $< -o $@

include/$(PROTO).fbs.hpp: protos/$(PROTO).fbs
	make proto


.PHONY: clean
clean:
	rm -rf  bin/ obj/
	rm -f   include/?*.fbs.hpp

.PHONY: git
git:
	@# 不需要 clean, 因为编译产物已经被 gitignore'd 了.
	@# make clean
	git commit -av
	git push

.PHONY: proto
proto:
	flatc --gen-mutable --reflect-names --cpp protos/$(PROTO).fbs
	[ $$? -eq 0 ]  &&  mv $(PROTO)_generated.h include/$(PROTO).fbs.hpp
