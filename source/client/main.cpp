#include "pch.hpp" // IWYU pragma: keep

#include "Client/Client.hpp"

int main(int ac, char *av[]) {
  if (ac != 2) {
    std::cerr << "[Error] - Main - Configuration file path not given!"
              << std::endl;
    return 1;
  }
  Client &instance = Client::Instance();

  if (!instance.setupClient(av[1])) {
    return 1;
  }
  if (!instance.run()) {
    return 1;
  }
  return 0;
}

// #include "common/Utils.hpp"
// #include <readline/readline.h>
//
// static void handler(char *line);
// static bool quit = false;
//
// int main(void) {
//   // Affiche le prompt et lit la ligne
//   rl_callback_handler_install(">>> ", &handler);
//   while (!quit) {
//     rl_callback_read_char();
//   }
// }
//
// static void handler(char *line) {
//   if (!line) {
//     std::cout << "EMPTY" << std::endl;
//     return;
//   }
//
//   std::string input(line);
//
//   // Trim et traitement de la ligne
//   if (!input.empty()) {
//     std::cout << "You entered: " << input << std::endl;
//   } else {
//     std::cout << "You entered nothing asshole " << input << std::endl;
//   }
//   Utils::trim(input);
//   if (input == "quit") {
//     quit = true;
//   }
//
//   free(line); // readline alloue dynamiquement la mémoire
// }
