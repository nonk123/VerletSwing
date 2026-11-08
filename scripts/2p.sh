#!/usr/bin/env bash

./build/VerletSwing &
sleep 0.5
./build/VerletSwing &

wait
