/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSTelemedProbe.h"
#include "mitkUSTelemedSDKHeader.h"

mitk::USTelemedProbe::USTelemedProbe(Usgfw2Lib::IProbe* probe, Usgfw2Lib::IUsgDataView* dataView)
  : m_UsgProbe(probe), m_UsgDataView(dataView)
{
  BSTR probeName;
  probe->get_Name(&probeName);
  SetName(telemed::ConvertWcharToString(probeName));
}

mitk::USTelemedProbe::~USTelemedProbe()
{
  SAFE_RELEASE(m_UsgProbe);
  SAFE_RELEASE(m_UsgDataView);
}

Usgfw2Lib::IProbe* mitk::USTelemedProbe::GetUsgProbe()
{
  return m_UsgProbe;
}

Usgfw2Lib::IUsgDataView* mitk::USTelemedProbe::GetUsgDataView()
{
  return m_UsgDataView;
}
