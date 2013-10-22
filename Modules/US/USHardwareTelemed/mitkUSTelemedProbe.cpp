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