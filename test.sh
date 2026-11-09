echo -e "\033[1;32m\nIntiating test...\n\033[0m"

#header compile test
echo -e "\033[0;33m[Header] Starting header test...\033[0m"
g++ -I src/ws/components/bridge/include\
    -I src/ws/components/cmd/include\
    -I src/ws/components/interface/include\
    -I src/ws/components/msg/include\
    -I src/ws/components/port/include\
    -I src/ws/components/sensors/include\
    -I src/ws/components/system/include\
    -I src/ws/components/telemetry/include\
    -I src/ws/components/test/include\
    -I src/ws/components/utils/include\
    src/ws/test/header_test.cpp -o src/ws/test/trash/header_test\
     && echo -e "\033[0;33m[Header] All headers compiled successfully.\033[0m\n"

#TODO other tests

echo -e "\nAll tests finished."
echo -e "Cleaning directory..."
rm -rf src/refactor/test/trash/*
echo -e "Finished cleaning directory."
echo -e "\033[1;32mTest script finished.\n\033[0m"
