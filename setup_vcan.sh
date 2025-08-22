#!/bin/bash
if ! ip link show vcan0 &> /dev/null; then
    echo "🔧 Création de l'interface vcan0..."
    sudo modprobe vcan
    sudo modprobe can_raw
    sudo ip link add dev vcan0 type vcan
    sudo ip link set up vcan0
    echo "✅ vcan0 créé avec succès"
fi
