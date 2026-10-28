echo -e "\033[1;32m\nIntiating test...\n\033[0m"

#header compile test
echo -e "\033[0;33m[Header] Starting header test...\033[0m"
g++ -I src/refactor/components/bridge/include\
    -I src/refactor/components/cmd/include\
    -I src/refactor/components/interface/include\
    -I src/refactor/components/msg/include\
    -I src/refactor/components/port/include\
    -I src/refactor/components/sensors/include\
    -I src/refactor/components/system/include\
    -I src/refactor/components/telemetry/include\
    -I src/refactor/components/test/include\
    -I src/refactor/components/utils/include\
    src/refactor/test/header_test.cpp -o src/refactor/test/trash/header_test && echo -e "\033[0;33m[Header] All headers compiled successfully.\033[0m\n"

echo -e "\nAll tests finished."
echo -e "Cleaning directory..."
rm -rf src/refactor/test/trash/*
echo -e "Finished cleaning directory."
echo -e "\033[1;32mTest script finished.\n\033[0m"
