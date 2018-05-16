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

#using "Interop.OphirLMMeasurementLib.dll"
#include <vector>
#include <msclr\auto_gcroot.h>
#include "OphirPyroWrapper.h"

using namespace System::Runtime::InteropServices; // Marshal

class OphirPyroWrapperPrivate
{
  public: msclr::auto_gcroot<OphirLMMeasurementLib::CoLMMeasurement^> ophirAPI;
};

OphirPyroWrapper::OphirPyroWrapper()
{
  _private = new OphirPyroWrapperPrivate();
  _private->ophirAPI = gcnew OphirLMMeasurementLib::CoLMMeasurement();
}

OphirPyroWrapper::~OphirPyroWrapper()
{
  delete _private;
}

char* OphirPyroWrapper::ScanUSB()
{
  char* foo;
  System::Object^ managedObject;
  _private->ophirAPI->ScanUSB(managedObject);
  array<System::String^>^ managedCapi = dynamic_cast<array<System::String^>^>(managedObject);
  if (managedCapi->Length != 0)
    return (char*)Marshal::StringToHGlobalAnsi(managedCapi[0]).ToPointer();
  else
    return 0;
}

int OphirPyroWrapper::OpenDevice(char* serialNumber)
{
  int deviceHandle;
  _private->ophirAPI->OpenUSBDevice(gcnew System::String(serialNumber), deviceHandle);
  
  return deviceHandle;
}

char* OphirPyroWrapper::GetWavelengths(int deviceHandle)
{
  int index;
  System::Object^ options;
  // Show wavelengths of channel 0
  _private->ophirAPI->GetWavelengths(deviceHandle, 0, index, options);
  array<System::String^>^ managedCapi = dynamic_cast<array<System::String^>^>(options);
  if (managedCapi->Length != 0)
    return (char*)Marshal::StringToHGlobalAnsi(managedCapi[index]).ToPointer();
  else
    return 0;
}

char* OphirPyroWrapper::GetRanges(int deviceHandle)
{
  int index;
  System::Object^ options;
  // Show ranges of channel 0
  _private->ophirAPI->GetRanges(deviceHandle, 0, index, options);
  array<System::String^>^ managedCapi = dynamic_cast<array<System::String^>^>(options);
  if (managedCapi->Length != 0)
    return (char*)Marshal::StringToHGlobalAnsi(managedCapi[index]).ToPointer();
  else
    return 0;
}

bool OphirPyroWrapper::StartStream(int deviceHandle)
{
  _private->ophirAPI->StartStream(deviceHandle, 0);
  return true;
}
  
bool OphirPyroWrapper::StopStream(int deviceHandle)
{
  _private->ophirAPI->StopStream(deviceHandle, 0);
  return true;
}

bool OphirPyroWrapper::CloseDevice(int deviceHandle)
{
  _private->ophirAPI->Close(deviceHandle);
  return true;
}

unsigned int OphirPyroWrapper::GetData(int deviceHandle, std::vector<double>* data, std::vector<double>* timestamp, std::vector<int>* status)
{
  System::Object^ dataArray;
  System::Object^ timeStampArray;
  System::Object^ statusArray;
  array<double>^ managedDataArray;
  array<double>^ managedTimeStampArray;
  array<int>^ managedStatusArray;
  _private->ophirAPI->GetData(deviceHandle, 0, dataArray, timeStampArray, statusArray);
  managedDataArray = (array<double>^)dataArray;
  managedTimeStampArray = (array<double>^)timeStampArray;
  managedStatusArray = (array<int>^)statusArray;
  if (managedDataArray->Length > 0)
  {
    data->resize(managedDataArray->Length);
    timestamp->resize(managedDataArray->Length);
    status->resize(managedDataArray->Length);
    for(int i = 0; i<managedDataArray->Length; i++)
    {
      (*data)[i] = managedDataArray[i];
      (*timestamp)[i] = managedTimeStampArray[i];
      (*status)[i] = managedStatusArray[i];
      // DEBUG: std::cout << "managedDataArray " << i << ": " << managedDataArray[i] << " ts: " << managedTimeStampArray[i] << " status: " << managedStatusArray[i] << std::endl;  
    }
    return managedDataArray->Length;
  }
  else
  {
    data->resize(1);
    timestamp->resize(1);
    status->resize(1);
    (*data)[0] = -1;
    (*timestamp)[0] = -1;
    (*status)[0] = -1;
  }

  return 0;
}