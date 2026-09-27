#include <iostream>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Msg.h"
#include "MsgESPNOW.h"
#include "Nodes.h"
#include "MCU.h"
#include "Test.h"

//CONFIG
//esp0 build
constexpr MCU::ID THIS_MCU = MCU::ID::ESP0;
constexpr MCU::ID PEER_MCU = MCU::ID::ESP1;

//esp1 build
//constexpr MCU::ID THIS_MCU = MCU::ID::ESP1;
//constexpr MCU::ID PEER_MCU = MCU::ID::ESP0;

constexpr int PACKETS_TO_SEND = 100;

constexpr int SEND_RETRIES = 100;
constexpr int GET_RETRIES = 500;

constexpr TickType_t RETRY_DELAY = pdMS_TO_TICKS(10);


//TEST
bool testESPNOW()
{
  Msg::Node node(Node::Address{THIS_MCU, Node::ID::MSG_ESPNOW_TEST});
  const Node::Address peer{PEER_MCU, Node::ID::MSG_ESPNOW_TEST};

  int send_success = 0;
  int send_fail = 0;

  int get_success = 0;
  int get_fail = 0;

  int data_errors = 0;

  for(int i = 0; i < PACKETS_TO_SEND; i++){
    //parse msg
    Test::MsgTest message{};
    
    message.header = i;
    message.test_uint8 = static_cast<uint8_t>(i);
    message.test_float = static_cast<float>(i) + 0.5f;

    //send
    bool sent = false;
    for (int retry = 0; retry < SEND_RETRIES; retry++) {
      if(node.send(message, peer)){
        sent = true;
        break;
      }
      send_fail++;
      vTaskDelay(RETRY_DELAY);
    }

    if (!sent) {
      std::cout << "[ESP-NOW] SEND FAILED"
                << " packet=" << i
                << "\n";
    }
    else send_success++;
        
        
    //receive    
    Test::MsgTest received{};
    bool received_ok = false;

    for(int retry = 0; retry < GET_RETRIES; retry++){
      if(node.get(received)){
        received_ok = true;
        break;
      }
      vTaskDelay(RETRY_DELAY);
    }

    if(!received_ok) {
      get_fail++;
      std::cout << "[ESP-NOW] RECEIVE FAILED"
                << " packet=" << i
                << "\n";
    }
    else get_success++;

    //data validation
    uint8_t expected_uint8 = static_cast<uint8_t>(received.header);
    float expected_float = static_cast<float>(received.header) + 0.5f;

    if (received.test_uint8 != expected_uint8){
      data_errors++;
      std::cout << "[ESP-NOW] UINT8 ERROR"
                << " i=" << i
                << " expected=" << static_cast<int>(expected_uint8)
                << " received=" << static_cast<int>(received.test_uint8)
                << "\n";
    }

    if (received.test_float != expected_float) {
      data_errors++;

      std::cout << "[ESP-NOW] FLOAT ERROR"
                << " i=" << i
                << " expected=" << expected_float
                << " received=" << received.test_float
                << "\n";
    }
  }

  //results
  std::cout << "\n[ESP-NOW] INTER-ESP TEST\n"
            << "      This MCU       :" << static_cast<int>(THIS_MCU) << "\n"
            << "      Peer MCU       :" << static_cast<int>(PEER_MCU) << "\n"
            << "      Packets        :" << PACKETS_TO_SEND << "\n"
            << "      Data errors    :" << data_errors << "\n"
            << "      Send success   :" << send_success << "\n"
            << "      Send retries   :" << send_fail << "\n"
            << "      Get success    :" << get_success << "\n"
            << "      Get failures   :" << get_fail + (PACKETS_TO_SEND-get_success) << "\n"
            << "      Send success(%):" << float(send_success)/PACKETS_TO_SEND*100 << "\n"
            << "      Get success(%) :" << float(get_success)/PACKETS_TO_SEND*100 << "\n";
            

  bool passed = send_success == PACKETS_TO_SEND &&
                get_success == PACKETS_TO_SEND &&
                data_errors == 0;


  std::cout << "      Result         :" << (passed ? "PASS" : "FAIL") << "\n";
  return passed;
}


//MAIN
int main()
{
  std::cout << "[ESP-NOW] Initializing...\n";
  if (!Msg::detail::beginESPNOW()) {
    std::cout << "[ESP-NOW] Initialization failed.\n";
    return 1;
  }


  std::cout << "[ESP-NOW] Initialization successful.\n";
  bool passed = testESPNOW();

  if(passed){
    std::cout << "[ESP-NOW] TEST PASSED\n";
    return 0;
  }

  std::cout << "[ESP-NOW] TEST FAILED\n";
  return 1;
}

extern "C" void app_main()
{
    const int result = main();

    std::cout.flush();
    fflush(stdout);

    printf("\n[ESP] msg_test returned %d\n", result);
}
