echo -e "\033[1;32m\nIntiating test...\n\033[0m"

#header compile test
echo -e "\033[0;33m[Header] Starting header test...\033[0m"
g++ -I src/ws/include src/ws/test/header_test.cpp -o src/ws/test/trash/header_test && echo -e "\033[0;33m[Header] All headers compiled successfully.\033[0m\n"

#msg test

#Msg.cpp line 30 comment out sendESPNOWPacket(packet)
#to avoid compile error

echo -e "\033[0;33m[MSG] Starting message test...\033[0m"
g++ -pthread -I src/ws/include src/ws/msg/Msg.cpp src/ws/test/msg_test.cpp -o src/ws/test/trash/msg_test && echo -e "\033[0;33m[MSG] All test codes compiled successfully.\033[0m"
./src/ws/test/trash/msg_test


echo -e "\nAll tests finished."
echo -e "Cleaning directory..."
rm -rf src/ws/test/trash/*
echo -e "Finished cleaning directory."
echo -e "\033[1;32mTest script finished.\n\033[0m"
