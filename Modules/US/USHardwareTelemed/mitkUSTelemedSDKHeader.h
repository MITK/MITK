/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSTelemedSDKHeader_H
#define MITKUSTelemedSDKHeader_H

/*#include <strmif.h>
#include <usgfw2.h>
#include <usgfw.h>
#include <usgscanb.h>*/

#include <Usgfw2.tlh>

#include <mitkCommon.h>

/**
  * Release the given COM object pointer and set the pointer to null.
  */
#define SAFE_RELEASE(x) { if (x) x->Release(); x = nullptr; }

/**
  * Get the current value from the given COM object and return it as double.
  */
#define RETURN_TelemedValue(control) { \
  LONG value; \
  HRESULT hr = control->get_Current(&value); \
  if (FAILED(hr)) { mitkThrow() << "Could not get telemed value " << control << "(" << hr << ")."; }; \
  return static_cast<double>(value); \
}

/**
  * Cast the value to LONG and set it at the given COM object.
  */
#define SET_TelemedValue(control,value) { \
  HRESULT hr = control->put_Current(static_cast<LONG>(value)); \
  if (FAILED(hr)) { mitkThrow() << "Could not set telemed value " << value << " to " << control << "(" << hr << ")."; }; \
}

/**
  * Get all available values for given COM object and return
  * them as std::vector<double>.
  */
#define RETURN_TelemedAvailableValues(control) { \
  RETURN_TelemedAvailableValuesWithFactor(control, 1); \
}

#define RETURN_TelemedAvailableValuesWithFactor(control, factor) { \
  Usgfw2Lib::IUsgValues *usgValues; \
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
    item = usgValues->Item(n); \
    values.at(n) = static_cast<double>(item.lVal) / factor; \
    VariantClear(&item); \
  } \
  \
  SAFE_RELEASE(usgValues); \
  return values; \
}

/**
  * Get all available values for given COM object, calculate
  * minimum, maximum and interval step from them and save this
  * three in given "output" variable. This variable must be a
  * c array with three elements.
  */
#define GETINOUTPUT_TelemedAvailableValuesBounds(control, output) { \
  Usgfw2Lib::IUsgValues *usgValues; \
  HRESULT hr = control->get_Values(&usgValues); \
  if (FAILED(hr)) { mitkThrow() << "Values couldn't be read from Teleme API (" << hr << ")."; } \
  \
  LONG usgValuesNum; \
  hr = usgValues->get_Count(&usgValuesNum); \
  if (usgValuesNum < 1 || FAILED(hr)) { mitkThrow() << "No values could be read from Telemed API."; } \
  \
  VARIANT item; \
  \
  item = usgValues->Item(0); \
  output[0] = static_cast<double>(item.lVal); \
  VariantClear(&item); \
  \
  item = usgValues->Item(usgValuesNum-1); \
  output[1] = static_cast<double>(item.lVal); \
  VariantClear(&item); \
  \
  output[2] = (output[1] - output[0]) / usgValuesNum; \
}

/**
  * Create Telemed API control. The interface documentation can be found
  * in the Telemed API documentation regarding CreateUsgControl().
  */
#define CREATE_TelemedControl(control, dataView, iidType, type, scanMode) { \
  IUnknown* tmp_obj = nullptr; \
  mitk::telemed::CreateUsgControl( dataView, iidType, scanMode, 0, (void**)&tmp_obj ); \
  if ( ! tmp_obj ) { mitkThrow() << "Could not create telemed control " << control << ")."; } \
  \
  SAFE_RELEASE(control); \
  control = (type*)tmp_obj; \
}

namespace mitk {
  namespace telemed {
    enum ScanModes { ScanModeB };

    /**
      * Just a helper function for avoiding code duplication when creating
      * a Telemed API control. This function is from the Telemed API documentation
      * thus a description of the interface can be found there.
      */
    bool CreateUsgControl( Usgfw2Lib::IUsgDataView* dataView, const IID& typeId, ULONG scanMode, ULONG streamId, void** ctrl );

    /**
      * Converts given BSTR (which is in fact a wchar*) to std::string.
      */
    std::string ConvertWcharToString( const BSTR input );
  } // namespace telemed
} // namespace mitk

#endif // MITKUSTelemedSDKHeader_H
