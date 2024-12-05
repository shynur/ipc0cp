SHELL = bash
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -ggdb -g3  \
		   -fconcepts  \
		   -Iinclude  \
           -Wno-pointer-arith -Wno-return-type -Wno-unused-parameter
LDFLAGS = -lpthread -lrt

READERS = reader-1
PROTO = laser

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin


.PHONY: test
.SILENT: test
test:  bin/writer.exe bin/$(READERS).exe
	echo
	for reader in $(READERS); do  \
	    bin/$$reader.exe &        \
	done
	bin/writer.exe

bin/writer.exe:  obj/writer.o
	mkdir -p bin
	$(CXX) $^ $(LDFLAGS) -o $@

bin/$(READERS).exe:  obj/$(READERS).o
	mkdir -p bin
	$(CXX) $^ $(LDFLAGS) -o $@

obj/writer.o:  src/writer.cpp include/ipc0cp.hpp include/$(PROTO)-by-flatc.hpp
	mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $< -o $@

obj/reader-%.o: src/reader-%.cpp include/ipc0cp.hpp include/$(PROTO)-by-flatc.hpp
	mkdir -p obj
	$(CXX) -c $(CXXFLAGS) $< -o $@

include/$(PROTO)-by-flatc.hpp: protos/$(PROTO).fbs
	make proto

.PHONY: clean
clean:
	rm -rf  bin/ obj/
	rm -f   include/?*-by-flatc.hpp

.PHONY: git
git:
	@# 不需要 clean, 因为编译产物已经被 gitignore'd 了.
	@# make clean
	git commit -a -v
	git push

.PHONY: proto
proto:
	flatc --gen-mutable --reflect-names --cpp protos/$(PROTO).fbs
	[ $$? -eq 0 ]  &&  mv $(PROTO)_generated.h include/$(PROTO)-by-flatc.hpp
