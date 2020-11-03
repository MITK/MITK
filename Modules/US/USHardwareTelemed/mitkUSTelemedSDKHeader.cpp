/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSTelemedSDKHeader.h"

#include <mitkCommon.h>

/*bool mitk::telemed::CreateUsgControl( Usgfw2Lib::IUsgDataView* dataView, const IID& typeId, ULONG scanMode, ULONG streamId, void** ctrl )
{
  if ( ! dataView ) {
    MITK_WARN("CreateUsgControl") << "DataView must not be null.";
    return false;
  }

  Usgfw2Lib::IUsgControl* ctrl2 = nullptr;

  HRESULT hr;
  hr = dataView->GetControlObj( (GUID*)(&typeId), scanMode, streamId, &ctrl2 );
  if (FAILED(hr) || ! ctrl2)
  {
    MITK_WARN("CreateUsgControl") << "Could not get control object from data view (" << hr << ").";
    return false;
  }

  hr = ctrl2->QueryInterface( typeId, (void**)ctrl );
  if (FAILED(hr)) { *ctrl = nullptr; }

  SAFE_RELEASE(ctrl2);

  return true;
}*/

bool mitk::telemed::CreateUsgControl( Usgfw2Lib::IUsgDataView* data_view, const IID& type_id, ULONG scan_mode, ULONG
                                  stream_id, void** ctrl )
{
  Usgfw2Lib::IUsgControl* ctrl2;
  ctrl2 = nullptr;
  if (data_view == nullptr) return false;
  data_view->GetControlObj( (GUID*)(&type_id), scan_mode, stream_id, &ctrl2 );
  if (ctrl2 != nullptr)
  {
    HRESULT hr;
    hr = ctrl2->QueryInterface( type_id, (void**)ctrl );
    if (hr != S_OK) *ctrl = nullptr;
    SAFE_RELEASE(ctrl2);
  }

  return true;
}

std::string mitk::telemed::ConvertWcharToString( const BSTR input )
{
  // BSTR (-> wchar*) must first be converted to a std::wstring
  std::wstring tmp(input);

  // this can be converted to a std::string then
  std::string output(tmp.begin(), tmp.end());

  return output;
}
