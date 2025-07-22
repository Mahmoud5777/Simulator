# Nom de l'exécutable
TARGET = simulator

# Compilateur
CXX = g++

# Options de compilation
CXXFLAGS = -Wall -std=c++17

# Fichiers sources
SRCS = main.cpp \
       SimillatorController.cpp \
       EcuSender.cpp \
       EcuReceiver.cpp \
       Ecu.cpp \
       CanManager.cpp \
       BusManager.cpp \
       FrameCAN.cpp \
       FrameCanTP.cpp

# Fichiers objets générés automatiquement
OBJS = $(SRCS:.cpp=.o)

# Règle par défaut
all: $(TARGET)

# Construction de l'exécutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Nettoyage
clean:
	rm -f $(OBJS) $(TARGET)
