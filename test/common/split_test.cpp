#include "catch2/catch_test_macros.hpp"

#include "common/Utils.hpp"

TEST_CASE("split - basic delimiter usage", "[split]") {
  std::string input = "one##two##three";
  std::string sep = "##";

  auto result = Utils::split(input, sep);

  REQUIRE(result.size() == 3);
  REQUIRE(result[0] == "one");
  REQUIRE(result[1] == "two");
  REQUIRE(result[2] == "three");
}

TEST_CASE("split - delimiter at the end", "[split]") {
  std::string input = "one##two##";
  auto result = Utils::split(input, "##");

  REQUIRE(result.size() == 2);
  REQUIRE(result[0] == "one");
  REQUIRE(result[1] == "two");
}

TEST_CASE("split - delimiter at the beginning", "[split]") {
  std::string input = "##one##two";
  auto result = Utils::split(input, "##");

  REQUIRE(result.size() == 2);
  REQUIRE(result[0] == "one");
  REQUIRE(result[1] == "two");
}

TEST_CASE("split - multiple consecutive delimiters", "[split]") {
  std::string input = "one####two";
  auto result = Utils::split(input, "##");

  REQUIRE(result.size() == 2); // the "" between is ignored
  REQUIRE(result[0] == "one");
  REQUIRE(result[1] == "two");
}

TEST_CASE("split - no delimiter", "[split]") {
  std::string input = "hello world";
  auto result = Utils::split(input, "##");

  REQUIRE(result.size() == 1);
  REQUIRE(result[0] == "hello world");
}

TEST_CASE("split - only delimiter", "[split]") {
  std::string input = "##";
  auto result = Utils::split(input, "##");

  REQUIRE(result.empty());
}

TEST_CASE("split - empty string", "[split]") {
  std::string input = "";
  auto result = Utils::split(input, "##");

  REQUIRE(result.empty());
}
