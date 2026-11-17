#include <iostream>
#include <cstdint>
#include <thread>
#include <atomic>

#include "Msg.h"
#include "Nodes.h"
#include "MCU.h"

#include "Test.h"

//=========================
//concurrent messaging test
//=========================
constexpr int PACKETS_PER_NODE = 1000;
constexpr int QUEUE_CAPACITY = 128;

struct TestResult {
  int send_success = 0;
  int send_fail = 0;

  int get_success = 0;
  int get_fail = 0;

  int data_errors = 0;
};


void node0_task(TestResult& result){
  Msg::Node node0(MCU::TEST.address<Node::ID::MSG_LOCAL_NODE0_TEST>());

  for (int i = 0; i < PACKETS_PER_NODE; i++) {
    //create packet
    Test::MsgTest message{};
    
    message.header = i;
    message.test_uint8 = static_cast<uint8_t>(i);
    message.test_float = static_cast<float>(i) + 0.5f;

    //keep trying until the packet enters queue
    while (!node0.send(message, MCU::TEST.address<Node::ID::MSG_LOCAL_NODE1_TEST>())) {
      result.send_fail++;
      std::this_thread::yield();
    }
    result.send_success++;

    //wait for node1 response
    Test::MsgTest received{};

    while (!node0.get(received)) {std::this_thread::yield();}
    result.get_success++;


    //node1 sends 100-149
    uint8_t expected_uint8 = static_cast<uint8_t>(i + 100);
    float expected_float = static_cast<float>(i) + 100.5f;
        
    if (received.test_uint8 != expected_uint8) result.data_errors++;
    if (received.test_float != expected_float) result.data_errors++;
  }
}


void node1_task(TestResult& result){
  Msg::Node node1(MCU::TEST.address<Node::ID::MSG_LOCAL_NODE1_TEST>());

  for(int i = 0; i < PACKETS_PER_NODE; i++){
    //create packet
    Test::MsgTest message{};

    message.test_uint8 = static_cast<uint8_t>(i + 100);
    message.test_float = static_cast<float>(i) + 100.5f;


    //keep trying until the packet enters queue
    while (!node1.send(message, MCU::TEST.address<Node::ID::MSG_LOCAL_NODE0_TEST>())) {
      result.send_fail++;
      std::this_thread::yield();
    }
    result.send_success++;

    //wait for node0 response.
    Test::MsgTest received{};

    while (!node1.get(received)) {std::this_thread::yield();}
    result.get_success++;


    //node0 sends 0-49.
    uint8_t expected_uint8 = static_cast<uint8_t>(i);
    float expected_float = static_cast<float>(i) + 0.5f;

    if (received.test_uint8 != expected_uint8) result.data_errors++;
    if (received.test_float != expected_float) result.data_errors++;
  }
}

bool testConcurrent(){
  TestResult node0_result{};
  TestResult node1_result{};

  std::thread node0_thread(node0_task, std::ref(node0_result));
  std::thread node1_thread(node1_task, std::ref(node1_result));

  node0_thread.join();
  node1_thread.join();

  int total_send_success = node0_result.send_success + node1_result.send_success;
  int total_send_fail = node0_result.send_fail + node1_result.send_fail;
  int total_get_success = node0_result.get_success + node1_result.get_success;
  int total_get_fail = node0_result.get_fail + node1_result.get_fail;
  int total_data_errors = node0_result.data_errors + node1_result.data_errors;

  std::cout << "[MSG] CONCURRENT MESSAGE TEST\n"
            << "      Packets per node: " << PACKETS_PER_NODE << "\n"
            << "      Send success:     " << total_send_success << "\n"
            << "      Send retries:     " << total_send_fail << "\n"
            << "      Get success:      " << total_get_success << "\n"
            << "      Get failures:     " << total_get_fail << "\n"
            << "      Data errors:      " << total_data_errors << "\n";

  bool passed = total_send_success == PACKETS_PER_NODE * 2 &&
                total_get_success == PACKETS_PER_NODE * 2 &&
                total_data_errors == 0;

  std::cout << "      Result:           " << (passed ? "PASS" : "FAIL") << "\n";
  return passed;
}


//=====================
//queue saturation test
//to test if queue overflow doesn't happen and memory stays intact
//=====================
bool testQueueFull() {
  Msg::Node sender(MCU::TEST.address<Node::ID::MSG_LOCAL_NODE0_TEST>());
  Msg::Node receiver(MCU::TEST.address<Node::ID::MSG_LOCAL_NODE1_TEST>());


  //fill queue completely.
  int successful_sends = 0;
  for (int i = 0; i < QUEUE_CAPACITY; i++) {
    Test::MsgTest message{};

    message.test_uint8 = static_cast<uint8_t>(i);
    message.test_float = static_cast<float>(i);

    if (sender.send(message, MCU::TEST.address<Node::ID::MSG_LOCAL_NODE1_TEST>()))
      successful_sends++;
  }


  //next packet should fail with queue full
  Test::MsgTest overflow_message{};
  bool overflow_send = sender.send(overflow_message,
                                   MCU::TEST.address<Node::ID::MSG_LOCAL_NODE1_TEST>());

  std::cout << "[MSG] QUEUE FULL TEST\n"
            << "      Queue capacity:   " << QUEUE_CAPACITY << "\n"
            << "      Successful sends: " << successful_sends << "\n"
            << "      Overflow send:    "
            << (overflow_send ? "unexpected success" : "correctly failed")
            << "\n";


  //consume everything in queue
  int successful_gets = 0;
  for (int i = 0; i < QUEUE_CAPACITY; i++){
    Test::MsgTest received{};
    if (!receiver.get(received)) break;
    successful_gets++;
  }

  std::cout << "      Successful gets:  " << successful_gets << "\n";

  bool passed = successful_sends == QUEUE_CAPACITY &&
                !overflow_send &&
                successful_gets == QUEUE_CAPACITY;

  std::cout << "      Result:           " << (passed ? "PASS" : "FAIL") << "\n";
  return passed;
}


//==========
//main
//==========
int main()
{
  bool concurrent_passed = testConcurrent();
  bool queue_passed = testQueueFull();


  std::cout << "[MSG] MESSAGE TESTS\n"
            << "      Concurrent test:  "
            << (concurrent_passed ? "PASS" : "FAIL")
            << "\n"
            << "      Queue full test:  "
            << (queue_passed ? "PASS" : "FAIL")
            << "\n";
    
  if (concurrent_passed && queue_passed) {
    std::cout << "\033[0;33m[MSG] All tests passed.\033[0m\n";
    return 0;
  }

  std::cout << "\033[0;33m[MSG] Test failed.\033[0m\n";
  return 1;
}
