CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -lrt

HEADERS = ipc0cp.hpp
WRITER_SRC = writer.cpp
READER_SRCS = reader-1.cpp  
EXECUTABLES = writer.exe $(basename $(READER_SRCS)).exe


.PHONY: test
.SILENT: test
test: $(EXECUTABLES)
	echo
	for reader in reader-?.exe; do  \
	    ./$$reader &                \
	done
	./writer.exe  ||  true

writer.exe: $(WRITER_SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

reader-%.exe: reader-%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@


.PHONY: clean
clean:
	rm -f $(EXECUTABLES)

.PHONY: git
git:
	# 不需要 clean, 因为编译产物已经被 gitignore'd 了.
	# make clean
	git commit -a -v
	git push
