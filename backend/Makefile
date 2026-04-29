CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = compiler
SRCS   = main.cpp tac.cpp optimizer.cpp
OBJS   = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) compiler.exe

run-c:
	./compiler ../samples/sample1.c ../frontend/output.json

run-tac:
	./compiler ../samples/sample1.tac ../frontend/output.json

.PHONY: all clean run-c run-tac