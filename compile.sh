#!/bin/bash

# Crée le dossier build s'il n'existe pas
mkdir -p build

echo "📦 Compilation du simulateur..."
g++ src/*.cpp -o build/runSimulator
if [ $? -eq 0 ]; then
    echo "✅ Simulateur compilé avec succès : build/runSimulator"
else
    echo "❌ Erreur lors de la compilation du simulateur"
    exit 1
fi

echo "🧪 Compilation des tests..."
g++ -std=c++17 -pthread \
    src/FrameCanTP.cpp src/CanManager.cpp src/BusManager.cpp tests/test.cpp \
    -Iinclude \
    -lgtest -lgtest_main \
    -o build/runTest

if [ $? -eq 0 ]; then
    echo "✅ Tests compilés avec succès : build/runTest"
else
    echo "❌ Erreur lors de la compilation des tests"
    exit 1
fi
