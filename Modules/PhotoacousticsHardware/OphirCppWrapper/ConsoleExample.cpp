/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <iostream>
#include <windows.h>
#include "OphirPyroWrapper.h"
#include <vector>

int main()
{
  OphirPyroWrapper pyro;

  char* sn = pyro.ScanUSB();
  if (sn != 0)
    std::cout << "sn: " << sn << std::endl;
  else
    std::cout << "sn: NULL" << std::endl;

  int handle = pyro.OpenDevice(sn);
  std::cout << "handle: " << handle << std::endl;

  char* wl = pyro.GetWavelengths(handle);
  std::cout << "wl: " << wl << std::endl;
  char* gr = pyro.GetRanges(handle);
  std::cout << "gr: " << gr << std::endl;

  bool start = pyro.StartStream(handle);
  std::cout << "start: " << start << std::endl;

  std::vector<double> data;
  std::vector<double> timestamp;
  std::vector<int> status;
  Sleep(100);
  unsigned int gd = pyro.GetData(handle, &data, &timestamp, &status);
  std::cout << "gd: " << gd << std::endl;
  std::cout << "length: " << data.size() << std::endl;
  for(int i = 0; i<gd; i++)
  {
    std::cout << "data " << i << ": " << data[i] << " ts: " << timestamp[i] << " status: " << status[i] << std::endl;  
  }

  Sleep(100);
  gd = pyro.GetData(handle, &data, &timestamp, &status);
  std::cout << "gd: " << gd << std::endl;
  std::cout << "length: " << data.size() << std::endl;
  for(int i = 0; i<gd; i++)
  {
    std::cout << "data " << i << ": " << data[i] << " ts: " << timestamp[i] << " status: " << status[i] << std::endl;  
  }
  
  
  Sleep(100);
  gd = pyro.GetData(handle, &data, &timestamp, &status);
  std::cout << "gd: " << gd << std::endl;
  std::cout << "length: " << data.size() << std::endl;
  for(int i = 0; i<gd; i++)
  {
    std::cout << "data " << i << ": " << data[i] << " ts: " << timestamp[i] << " status: " << status[i] << std::endl;  
  }

  Sleep(1000);
  gd = pyro.GetData(handle, &data, &timestamp, &status);
  std::cout << "gd: " << gd << std::endl;
  std::cout << "length: " << data.size() << std::endl;
  for(int i = 0; i<gd; i++)
  {
    std::cout << "data " << i << ": " << data[i] << " ts: " << timestamp[i] << " status: " << status[i] << std::endl;  
  }

  bool stop = pyro.StopStream(handle);
  std::cout << "stop: " << stop << std::endl;
  bool closed = pyro.CloseDevice(handle);
  std::cout << "closed: " << closed << std::endl;
}