/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSDiPhASProbe.h"
#include "Framework.IBMT.US.CWrapper.h"

mitk::USDiPhASProbe::USDiPhASProbe(std::string ProbeName)
{
	SetName(ProbeName);
}

mitk::USDiPhASProbe::~USDiPhASProbe()
{
}
