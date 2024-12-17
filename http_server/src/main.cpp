#include "Logger.h"

int main() {
  log::addConsole(TRACE);

  LOG(INFO) << "Hello World!";
}
