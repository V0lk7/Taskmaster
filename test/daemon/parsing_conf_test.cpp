#include "catch2/catch_test_macros.hpp"

#include "daemon/parsing/parsing.hpp"

int parsingTest(std::string config_file) {
  if (config_file.empty()) {
	return 1;
  }
  if (access(config_file.c_str(), R_OK) == -1) {
	return 1;
  }
  try {
    Daemon *daemon = parsingFile(config_file);
	delete daemon;
  }
  catch (const std::exception &e) {
	std::cerr << "Error: " << e.what() << std::endl;
	return 1;
  }
  return 0;
}

TEST_CASE("Parsing conf.yaml", "[parsing]") {
	REQUIRE(parsingTest("../../confs/conf.yaml") == 0);
}
TEST_CASE("Parsing conf_minimal.yaml", "[parsing]") {
	REQUIRE(parsingTest("../../confs/conf_minimal.yaml") == 0);
}
TEST_CASE("Parsing conf_noprog.yaml", "[parsing]") {
	REQUIRE(parsingTest("../../confs/conf_noprog.yaml") == 0);
}
TEST_CASE("Parsing conf_err_empty.yaml", "[parsing]") {
	REQUIRE(parsingTest("../../confs/conf_err_empty.yaml") == 1);
}
TEST_CASE("Parsing conf_err_noperms.yaml", "[parsing]") {
	REQUIRE(parsingTest("../../confs/conf_err_noperms.yaml") == 1);
}
TEST_CASE("Parsing conf_err_incomplete_prog.yaml", "[parsing]") {
	REQUIRE(parsingTest("../../confs/conf_err_incomplete_prog.yaml") == 1);
}
TEST_CASE("Empty parameter", "[parsing]") {
	REQUIRE(parsingTest("") == 1);
}
