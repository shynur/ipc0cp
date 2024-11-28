SHELL = bash
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0
LDFLAGS = -lrt -lprotobuf

HEADERS = ipc0cp.hpp
WRITER_SRC = writer.cpp
READER_SRCS = reader-1.cpp
PB_MSGS = laser
EXECUTABLES = writer.exe $(basename $(READER_SRCS)).exe


.PHONY: test
.SILENT: test
test: $(EXECUTABLES)
	echo
	for reader in reader-?.exe; do  \
	    ./$$reader &                \
	done
	./writer.exe  ||  true

writer.exe: writer.o $(PB_MSGS).pb.o
	$(CXX) $^ $(LDFLAGS) -o $@

reader-%.exe: reader-%.o $(PB_MSGS).pb.o
	$(CXX) $^ $(LDFLAGS) -o $@

writer.o: $(WRITER_SRC) $(HEADERS) $(PB_MSGS).pb.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

reader-%.o: reader-%.cpp $(HEADERS) $(PB_MSGS).pb.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.pb.h %.pb.cc: %.proto
	make protobuf

%.pb.o: %pb.cc %.pb.h
	$(CXX) -c $(CXXFLAGS) $< -o $@


.PHONY: clean
clean:
	rm -f $(EXECUTABLES)
	rm -f ?*.o
	rm -f ?*.pb.{h,cc}

.PHONY: git
git:
	@# 不需要 clean, 因为编译产物已经被 gitignore'd 了.
	@# make clean
	git commit -a -v
	git push

.PHONY: protobuf
protobuf:
	protoc --cpp_out=. ?*.proto
