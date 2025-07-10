#!/bin/bash

# Universal Backend Test Script
# Tests nlohmann/json, json11, and RapidJSON backends

echo "Universal Observable JSON - Backend Testing"
echo "============================================"

# Test nlohmann/json backend
echo "Testing nlohmann/json backend..."
rm -rf test_nlohmann
mkdir test_nlohmann
cd test_nlohmann

cmake -DUSE_JSON11=OFF -DUSE_RAPIDJSON=OFF -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1
make comprehensive_test > /dev/null 2>&1

echo "Running tests with nlohmann/json backend..."
./comprehensive_test
echo ""

cd ..

# Test json11 backend
echo "Testing json11 backend..."
rm -rf test_json11
mkdir test_json11
cd test_json11

cmake -DUSE_JSON11=ON -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1
make comprehensive_test > /dev/null 2>&1

echo "Running tests with json11 backend..."
./comprehensive_test
echo ""

cd ..

# Test RapidJSON backend
echo "Testing RapidJSON backend..."
rm -rf test_rapidjson
mkdir test_rapidjson
cd test_rapidjson

cmake -DUSE_RAPIDJSON=ON -DCMAKE_BUILD_TYPE=Release .. > /dev/null 2>&1
make comprehensive_test > /dev/null 2>&1

echo "Running tests with RapidJSON backend..."
./comprehensive_test
echo ""

cd ..

echo "Backend testing completed."
echo "Tested: nlohmann/json, json11, RapidJSON"

# Cleanup
rm -rf test_nlohmann test_json11 test_rapidjson
