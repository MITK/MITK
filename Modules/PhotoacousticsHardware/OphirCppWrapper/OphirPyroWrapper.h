/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <vector>

class OphirPyroWrapperPrivate;

class __declspec(dllexport) OphirPyroWrapper
{
  private: OphirPyroWrapperPrivate* _private;

  public: OphirPyroWrapper();

  public: ~OphirPyroWrapper();

  public: char* ScanUSB();

  public: int OpenDevice(char* serialNumber);

  public: char* GetWavelengths(int deviceHandle);

  public: char* GetRanges(int deviceHandle);

  public: bool StartStream(int deviceHandle);

  public: bool StopStream(int deviceHandle);

  public: bool CloseDevice(int deviceHandle);

  public: unsigned int GetData(int deviceHandle, std::vector<double>* data, std::vector<double>* timestamp, std::vector<int>* status);
};
