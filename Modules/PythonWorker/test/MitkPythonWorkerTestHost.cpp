/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonWorker.h>

#include <chrono>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

// Stands in for MITK in mitkPythonWorkerTest: keeps a worker that has a child
// process of its own, prints the process IDs of both and waits to be killed.
int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: MitkPythonWorkerTestHost <python> <test module>" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    std::ifstream file(argv[2]);
    std::ostringstream source;
    source << file.rdbuf();

    mitk::PythonWorker worker(argv[1], { { "mitk_worker_test", source.str() } }, "test host worker");

    const auto workerPid = worker.Call("mitk_worker_test.get_pid").get<int>();
    const auto childPid = worker.Call("mitk_worker_test.spawn_sleeper").get<int>();

    std::cout << workerPid << ' ' << childPid << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(1));
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
