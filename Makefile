CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
INCLUDES = -Iinclude

SRC = src/main.cpp src/client_handler.cpp src/logger.cpp \
      src/http_parser.cpp src/forwarder.cpp src/config.cpp \
      src/blocklist.cpp src/thread_pool.cpp src/server.cpp \
	  src/metrics.cpp


OUT = proxy

all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
