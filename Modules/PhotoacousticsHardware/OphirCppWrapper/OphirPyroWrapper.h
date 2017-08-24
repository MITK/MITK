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