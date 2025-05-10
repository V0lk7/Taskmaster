#include "catch2/catch_test_macros.hpp"

#include "common/Utils.hpp"

TEST_CASE("Utils::ltrim", "[ltrim]") {
  std::string s1 = "   hello";
  Utils::ltrim(s1);
  REQUIRE(s1 == "hello");

  std::string s2 = "no leading space";
  Utils::ltrim(s2);
  REQUIRE(s2 == "no leading space");

  std::string s3 = "    ";
  Utils::ltrim(s3);
  REQUIRE(s3 == "");
}

TEST_CASE("Utils::rtrim", "[rtrim]") {
  std::string s1 = "hello   ";
  Utils::rtrim(s1);
  REQUIRE(s1 == "hello");

  std::string s2 = "no trailing space";
  Utils::rtrim(s2);
  REQUIRE(s2 == "no trailing space");

  std::string s3 = "    ";
  Utils::rtrim(s3);
  REQUIRE(s3 == "");
}

TEST_CASE("Utils::trim", "[trim]") {
  std::string s1 = "   hello   ";
  Utils::trim(s1);
  REQUIRE(s1 == "hello");

  std::string s2 = "  already  trimmed  ";
  Utils::trim(s2);
  REQUIRE(s2 == "already  trimmed");

  std::string s3 = "no trim needed";
  Utils::trim(s3);
  REQUIRE(s3 == "no trim needed");

  std::string s4 = "   ";
  Utils::trim(s4);
  REQUIRE(s4 == "");
}
