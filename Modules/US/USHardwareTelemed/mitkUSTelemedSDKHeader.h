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

#ifndef MITKUSTelemedSDKHeader_H
#define MITKUSTelemedSDKHeader_H

#include <Usgfw2_i.c>   //wg IID's
#include <usgfw2.h>
#include <usgfw.h>
#include <usgscanb.h>
#include <comutil.h>    // for _bstr_t class

#include <mitkCommon.h>

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

#define RETURN_TelemedValue(control) { \
  LONG value; \
  HRESULT hr = control->get_Current(&value); \
  if (FAILED(hr)) { mitkThrow() << "Could not get telemed value " << control << "(" << hr << ")."; }; \
  return static_cast<double>(value); \
}

#define SET_TelemedValue(control,value) { \
  HRESULT hr = control->put_Current(static_cast<LONG>(value)); \
  if (FAILED(hr)) { mitkThrow() << "Could not set telemed value " << value << " to " << control << "(" << hr << ")."; }; \
}

#define RETURN_TelemedAvailableValues(control) { \
  IUsgValues *usgValues; \
  HRESULT hr = control->get_Values(&usgValues); \
  \
  LONG usgValuesNum; \
  hr = usgValues->get_Count(&usgValuesNum); \
  \
  std::vector<double> values(usgValuesNum, 0); \
  \
  VARIANT item; \
  for (int n = 0; n < usgValuesNum; n++) \
  { \
    usgValues->Item(n, &item); \
    values.at(n) = static_cast<double>(item.lVal); \
    VariantClear(&item); \
  } \
  \
  SAFE_RELEASE(usgValues); \
  return values; \
}

#define GETINOUTPUT_TelemedAvailableValuesBounds(control, output) { \
  IUsgValues *usgValues; \
  HRESULT hr = control->get_Values(&usgValues); \
  if (FAILED(hr)) { mitkThrow() << "Values couldn't be read from Teleme API (" << hr << ")."; } \
  \
  LONG usgValuesNum; \
  hr = usgValues->get_Count(&usgValuesNum); \
  if (usgValuesNum < 1 || FAILED(hr)) { mitkThrow() << "No values could be read from Telemed API."; } \
  \
  VARIANT item; \
  \
  usgValues->Item(0, &item); \
  output[0] = static_cast<double>(item.lVal); \
  VariantClear(&item); \
  \
  usgValues->Item(usgValuesNum-1, &item); \
  output[1] = static_cast<double>(item.lVal); \
  VariantClear(&item); \
  \
  output[2] = (output[1] - output[0]) / usgValuesNum; \
}

#define CREATE_TelemedControl(control, dataView, iidType, type, scanMode) { \
  IUnknown* tmp_obj = NULL; \
  mitk::telemed::CreateUsgControl( dataView, iidType, scanMode, 0, (void**)&tmp_obj ); \
  if ( ! tmp_obj ) { mitkThrow() << "Could not create telemed control " << control << ")."; } \
  \
  SAFE_RELEASE(control); \
  control = (type*)tmp_obj; \
}

namespace mitk {
  namespace telemed {

    enum ScanModes { ScanModeB };

    bool CreateUsgControl( IUsgDataView* dataView, const IID& typeId, ULONG scanMode, ULONG streamId, void** ctrl );
    std::string ConvertWcharToString( const BSTR input );
  }
}

#endif // MITKUSTelemedSDKHeader_H