CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS  = -lpthread

TARGET   = battle
SRCS     = main.cpp network.cpp game.cpp battle.cpp units.cpp protocol.cpp \
           ai_strategy.cpp state_sync.cpp discovery.cpp spectator.cpp battle_environment.cpp \
           team_network.cpp team_mode.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Dependencies
main.o: main.cpp network.h game.h battle.h protocol.h discovery.h state_sync.h spectator.h team_mode.h
network.o: network.cpp network.h
game.o: game.cpp game.h units.h protocol.h
battle.o: battle.cpp battle.h units.h ai_strategy.h state_sync.h spectator.h battle_environment.h
units.o: units.cpp units.h
protocol.o: protocol.cpp protocol.h units.h
ai_strategy.o: ai_strategy.cpp ai_strategy.h units.h
state_sync.o: state_sync.cpp state_sync.h units.h
discovery.o: discovery.cpp discovery.h network.h
spectator.o: spectator.cpp spectator.h
battle_environment.o: battle_environment.cpp battle_environment.h units.h
team_network.o: team_network.cpp team_network.h network.h
team_mode.o: team_mode.cpp team_mode.h team_network.h game.h battle.h protocol.h

.PHONY: all clean
