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

#include "mitkUSDiPhASBModeControls.h"
#include "mitkUSDiPhASDevice.h"
#include <mitkException.h>

#define DiPhAS_FREQUENCY_FACTOR 1000000

mitk::USDiPhASBModeControls::USDiPhASBModeControls(itk::SmartPointer<USDiPhASDevice> device)
: mitk::USControlInterfaceBMode(device.GetPointer()),
  m_Active(false), m_PowerSteps(new double[3]),
  m_GainSteps(new double[3]), m_RejectionSteps(new double[3]),
  m_DynamicRangeSteps(new double[3])
{
}

mitk::USDiPhASBModeControls::~USDiPhASBModeControls()
{
  //this->ReleaseControls();

  delete[] m_PowerSteps;
  delete[] m_GainSteps;
  delete[] m_RejectionSteps;
  delete[] m_DynamicRangeSteps;
}


/*void mitk::USDiPhASBModeControls::ReinitializeControls()
{
	this->ReleaseControls();
	this->CreateControls();
}
*/
void mitk::USDiPhASBModeControls::SetIsActive(bool active)
{
	if (active)
	{
		//this->CreateControls();
		m_Active = true;
	}
	else
	{
		//this->ReleaseControls();
		m_Active = false;
	}
}

bool mitk::USDiPhASBModeControls::GetIsActive()
{
	return m_Active;
}

double mitk::USDiPhASBModeControls::GetScanningFrequency()
{
	return 0;
}

void mitk::USDiPhASBModeControls::OnSetScanningFrequency(double value)
{
}

std::vector<double> mitk::USDiPhASBModeControls::GetScanningFrequencyValues()
{
	std::vector<double> h;
	h.push_back(3);
	return h;
}

double mitk::USDiPhASBModeControls::GetScanningPower()
{
	return 0;
}

void mitk::USDiPhASBModeControls::OnSetScanningPower(double value)
{
}

double mitk::USDiPhASBModeControls::GetScanningPowerMin()
{
	return m_PowerSteps[0];
}

double mitk::USDiPhASBModeControls::GetScanningPowerMax()
{
	return m_PowerSteps[1];
}

double mitk::USDiPhASBModeControls::GetScanningPowerTick()
{
	return m_PowerSteps[2];
}

double mitk::USDiPhASBModeControls::GetScanningDepth()
{
	return 0;
}

void mitk::USDiPhASBModeControls::OnSetScanningDepth(double value)
{
}

std::vector<double> mitk::USDiPhASBModeControls::GetScanningDepthValues()
{
	std::vector<double> h;
	h.push_back(3);
	return h;
}

double mitk::USDiPhASBModeControls::GetScanningGain()
{
	return 0;
}

void mitk::USDiPhASBModeControls::OnSetScanningGain(double value)
{
}

double mitk::USDiPhASBModeControls::GetScanningGainMin()
{
	return m_GainSteps[0];
}

double mitk::USDiPhASBModeControls::GetScanningGainMax()
{
	return m_GainSteps[1];
}

double mitk::USDiPhASBModeControls::GetScanningGainTick()
{
	return m_GainSteps[2];
}

double mitk::USDiPhASBModeControls::GetScanningRejection()
{
	return 0;
}

void mitk::USDiPhASBModeControls::OnSetScanningRejection(double value)
{
}

double mitk::USDiPhASBModeControls::GetScanningRejectionMin()
{
	return m_RejectionSteps[0];
}

double mitk::USDiPhASBModeControls::GetScanningRejectionMax()
{
	return m_RejectionSteps[1];
}

double mitk::USDiPhASBModeControls::GetScanningRejectionTick()
{
	return m_RejectionSteps[2];
}

double mitk::USDiPhASBModeControls::GetScanningDynamicRange()
{
	return 0;
}

void mitk::USDiPhASBModeControls::OnSetScanningDynamicRange(double value)
{
}

double mitk::USDiPhASBModeControls::GetScanningDynamicRangeMin()
{
	return m_DynamicRangeSteps[0];
}

double mitk::USDiPhASBModeControls::GetScanningDynamicRangeMax()
{
	return m_DynamicRangeSteps[1];
}

double mitk::USDiPhASBModeControls::GetScanningDynamicRangeTick()
{
	return m_DynamicRangeSteps[2];
}
