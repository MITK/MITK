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

#include "mitkUSTelemedSDKHeader.h"

#include <mitkCommon.h>

bool mitk::telemed::CreateUsgControl( IUsgDataView* dataView, const IID& typeId, ULONG scanMode, ULONG streamId, void** ctrl )
{
  if ( ! dataView ) {
    MITK_WARN("CreateUsgControl") << "DataView must not be null.";
    return false;
  }

  IUsgControl* ctrl2 = NULL;

  HRESULT hr;
  hr = dataView->GetControlObj( &typeId, scanMode, streamId, &ctrl2 );
  if (FAILED(hr) || ! ctrl2)
  {
    MITK_WARN("CreateUsgControl") << "Could not get control object from data view (" << hr << ").";
    return false;
  }

  hr = ctrl2->QueryInterface( typeId, (void**)ctrl );
  if (FAILED(hr)) { *ctrl = NULL; }

  SAFE_RELEASE(ctrl2);

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