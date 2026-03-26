CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS  = -lpthread

TARGET   = battle
SRCS     = main.cpp network.cpp game.cpp battle.cpp units.cpp protocol.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Dependencies
main.o: main.cpp network.h game.h battle.h protocol.h
network.o: network.cpp network.h
game.o: game.cpp game.h units.h protocol.h
battle.o: battle.cpp battle.h units.h
units.o: units.cpp units.h
protocol.o: protocol.cpp protocol.h units.h

.PHONY: all clean
