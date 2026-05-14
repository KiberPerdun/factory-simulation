//
// Created by KiberPerdun on 11.05.2026.
//

#include "simulator.h"

#include <iostream>

int
main (int32_t argc, char *argv[])
{
  if (argc < 2)
    {
      std::cerr << "Usage: " << argv[0] << " <input_file>\n";
      return 1;
    }

  std::string filename = argv[1];
  FactorySimulator sim;
  sim.run (filename);

  return 0;
}