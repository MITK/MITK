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

#include "mitkUSFraunhoferBModeControls.h"
#include "mitkUSFraunhoferDevice.h"
#include <mitkException.h>

#define Fraunhofer_FREQUENCY_FACTOR 1000000

mitk::USFraunhoferBModeControls::USFraunhoferBModeControls(itk::SmartPointer<USFraunhoferDevice> device)
: mitk::USControlInterfaceBMode(device.GetPointer()),
  m_Active(false), m_PowerSteps(new double[3]),
  m_GainSteps(new double[3]), m_RejectionSteps(new double[3]),
  m_DynamicRangeSteps(new double[3])
{
}

mitk::USFraunhoferBModeControls::~USFraunhoferBModeControls()
{
  //this->ReleaseControls();

  delete[] m_PowerSteps;
  delete[] m_GainSteps;
  delete[] m_RejectionSteps;
  delete[] m_DynamicRangeSteps;
}


/*void mitk::USFraunhoferBModeControls::ReinitializeControls()
{
	this->ReleaseControls();
	this->CreateControls();
}
*/
void mitk::USFraunhoferBModeControls::SetIsActive(bool active)
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

bool mitk::USFraunhoferBModeControls::GetIsActive()
{
	return m_Active;
}

double mitk::USFraunhoferBModeControls::GetScanningFrequency()
{
	return 0;
}

void mitk::USFraunhoferBModeControls::OnSetScanningFrequency(double value)
{
}

std::vector<double> mitk::USFraunhoferBModeControls::GetScanningFrequencyValues()
{
	std::vector<double> h;
	h.push_back(3);
	return h;
}

double mitk::USFraunhoferBModeControls::GetScanningPower()
{
	return 0;
}

void mitk::USFraunhoferBModeControls::OnSetScanningPower(double value)
{
}

double mitk::USFraunhoferBModeControls::GetScanningPowerMin()
{
	return m_PowerSteps[0];
}

double mitk::USFraunhoferBModeControls::GetScanningPowerMax()
{
	return m_PowerSteps[1];
}

double mitk::USFraunhoferBModeControls::GetScanningPowerTick()
{
	return m_PowerSteps[2];
}

double mitk::USFraunhoferBModeControls::GetScanningDepth()
{
	return 0;
}

void mitk::USFraunhoferBModeControls::OnSetScanningDepth(double value)
{
}

std::vector<double> mitk::USFraunhoferBModeControls::GetScanningDepthValues()
{
	std::vector<double> h;
	h.push_back(3);
	return h;
}

double mitk::USFraunhoferBModeControls::GetScanningGain()
{
	return 0;
}

void mitk::USFraunhoferBModeControls::OnSetScanningGain(double value)
{
}

double mitk::USFraunhoferBModeControls::GetScanningGainMin()
{
	return m_GainSteps[0];
}

double mitk::USFraunhoferBModeControls::GetScanningGainMax()
{
	return m_GainSteps[1];
}

double mitk::USFraunhoferBModeControls::GetScanningGainTick()
{
	return m_GainSteps[2];
}

double mitk::USFraunhoferBModeControls::GetScanningRejection()
{
	return 0;
}

void mitk::USFraunhoferBModeControls::OnSetScanningRejection(double value)
{
}

double mitk::USFraunhoferBModeControls::GetScanningRejectionMin()
{
	return m_RejectionSteps[0];
}

double mitk::USFraunhoferBModeControls::GetScanningRejectionMax()
{
	return m_RejectionSteps[1];
}

double mitk::USFraunhoferBModeControls::GetScanningRejectionTick()
{
	return m_RejectionSteps[2];
}

double mitk::USFraunhoferBModeControls::GetScanningDynamicRange()
{
	return 0;
}

void mitk::USFraunhoferBModeControls::OnSetScanningDynamicRange(double value)
{
}

double mitk::USFraunhoferBModeControls::GetScanningDynamicRangeMin()
{
	return m_DynamicRangeSteps[0];
}

double mitk::USFraunhoferBModeControls::GetScanningDynamicRangeMax()
{
	return m_DynamicRangeSteps[1];
}

double mitk::USFraunhoferBModeControls::GetScanningDynamicRangeTick()
{
	return m_DynamicRangeSteps[2];
}
