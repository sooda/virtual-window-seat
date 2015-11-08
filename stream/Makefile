CXXFLAGS = $(shell pkg-config --cflags --libs opencv) -std=c++11 -L /usr/local/cuda/targets/armv7-linux-gnueabihf/lib
stream: stream.cc
	$(CXX) $^ $(CXXFLAGS) -o $@
