#include "nanomsg/nn.h"
#include "nanomsg/pipeline.h"
#include "yaml-cpp/yaml.h"
#include <iostream>
#include <unistd.h>

int main(void) {
  std::cout << "TEST TEST" << std::endl;
  int sock = nn_socket(AF_SP, NN_PULL);

  if (sock > 0) {
    close(sock);
  }

  try {
    YAML::LoadAllFromFile("");
  } catch (...) {
    std::cout << "osef" << std::endl;
  }

  return 0;
}
