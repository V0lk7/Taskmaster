#include "Client/Client.hpp"
#include "catch2/catch_test_macros.hpp"

class ClientTest {
public:
  ClientTest() {
    // Setup code if needed
    Client &client = Client::Instance();

    Console::_processList = {"program1", "program2", "group1:program3",
                             "group1:program4"};
  }

  void setProcessList(std::vector<std::string> processList) {
    Client &client = Client::Instance();
    Console::_processList = processList;
  }

  void testProcessArgs(std::vector<std::string> &args,
                       std::vector<std::string> &expected) {
    Client &client = Client::Instance();
    auto processedArgs = client.processArgs(args);

    REQUIRE(processedArgs.size() == expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
      REQUIRE(processedArgs[i] == expected[i]);
    }
  }
};

TEST_CASE("Client::processArgs - basic usage", "[Client]") {
  ClientTest clientTest;

  // Setup a mock process list
  SECTION("Single argument") {
    std::vector<std::string> args = {"arg1"};
    std::vector<std::string> expected = {"arg1"};
    clientTest.testProcessArgs(args, expected);
  }

  SECTION("Multiple arguments") {
    std::vector<std::string> args = {"arg1", "arg2", "arg3"};
    std::vector<std::string> expected = {"arg1", "arg2", "arg3"};
    clientTest.testProcessArgs(args, expected);
  }

  SECTION("Empty arguments") {
    std::vector<std::string> args = {};
    std::vector<std::string> expected = {};
    clientTest.testProcessArgs(args, expected);
  }

  SECTION("Group with wildcard") {
    std::vector<std::string> args = {"group1:*"};
    std::vector<std::string> expected = {"group1:program3", "group1:program4"};
    clientTest.testProcessArgs(args, expected);
  }
}

TEST_CASE("Client::processArgs - edge cases", "[Client]") {
  ClientTest clientTest;

  // Setup a mock process list
  clientTest.setProcessList({"group1:program3", "group1:program4"});

  SECTION("Group with specific program") {
    std::vector<std::string> args = {"group1:program3"};
    std::vector<std::string> expected = {"group1:program3"};
    clientTest.testProcessArgs(args, expected);
  }

  SECTION("Group with wildcard and no matching processes") {
    std::vector<std::string> args = {"group2:*"};
    std::vector<std::string> expected = {};
    clientTest.testProcessArgs(args, expected);
  }

  SECTION("Mixed arguments with group and single programs") {
    std::vector<std::string> args = {"program1", "group1:*", "program2"};
    std::vector<std::string> expected = {"program1", "group1:program3",
                                         "group1:program4", "program2"};
    clientTest.testProcessArgs(args, expected);
  }
}
