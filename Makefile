SHELL = bash
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O0 -ggdb -g3  \
		   -fconcepts  \
           -Wno-pointer-arith -Wno-return-type -Wno-unused-parameter
LDFLAGS = -lpthread -lrt -lprotobuf -lcapnp -lkj -lcapnp-json

READERS = reader-1
MESSAGES = laser


.PHONY: test
.SILENT: test
test:  writer.exe $(READERS).exe
	echo
	for reader in $(READERS); do  \
	    ./$$reader.exe &          \
	done
	./writer.exe  ||  true

writer.exe:  writer.o               $(MESSAGES).pb.o $(MESSAGES).capnp.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(READERS).exe:  $(READERS).o       $(MESSAGES).pb.o $(MESSAGES).capnp.o
	$(CXX) $^ $(LDFLAGS) -o $@

writer.o:  writer.cpp ipc0cp.hpp    $(MESSAGES).pb.h $(MESSAGES).capnp.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

reader-%.o: reader-%.cpp ipc0cp.hpp $(MESSAGES).pb.h $(MESSAGES).capnp.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(MESSAGES).pb.h    $(MESSAGES).pb.cc: $(MESSAGES).proto
	protoc --cpp_out=. $<
$(MESSAGES).capnp.h $(MESSAGES).capnp.cpp:
	capnp compile -oc++ $(MESSAGES).capnp
	mv $(MESSAGES).capnp.{c++,cpp}


.PHONY: clean
clean:
	rm -f  ?*.exe
	rm -f  ?*.o
	rm -f  ?*.pb.{h,cc}  ?*.capnp.{h,c++}

.PHONY: git
git:
	@# 不需要 clean, 因为编译产物已经被 gitignore'd 了.
	@# make clean
	git commit -a -v
	git push

.PHONY: proto
proto:
	protoc --cpp_out=. ?*.proto
	capnp compile -oc++ ?*.capnp
