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

#include <string>
#include "mitkUSDiPhASProbesControls.h"
#include "mitkUSDiPhASDevice.h"
#include <mitkException.h>

mitk::USDiPhASProbesControls::USDiPhASProbesControls(itk::SmartPointer<USDiPhASDevice> device)
  : mitk::USControlInterfaceProbes(device.GetPointer()),
    m_IsActive(false), m_DiPhASDevice(device)
{
}

mitk::USDiPhASProbesControls::~USDiPhASProbesControls()
{
}


void mitk::USDiPhASProbesControls::SetIsActive(bool isActive)
{
	this->CreateProbesSet();
	m_IsActive = isActive;
}

bool mitk::USDiPhASProbesControls::GetIsActive()
{
	return m_IsActive;
}

std::vector<mitk::USProbe::Pointer> mitk::USDiPhASProbesControls::GetProbeSet()
{
	// create a new vector of base class (USProbe) objects, because
	// interface wants a vector of this type
	std::vector<mitk::USProbe::Pointer> usProbes(m_ProbesSet.size(), 0);
	for (unsigned int n = 0; n < m_ProbesSet.size(); ++n)
	{
		usProbes.at(n) = m_ProbesSet.at(n).GetPointer();
	}
	return usProbes;
}

void mitk::USDiPhASProbesControls::OnSelectProbe(unsigned int index)
{
	if (index >= m_ProbesSet.size())
	{
		MITK_ERROR("USDiPhASProbesControls")("USControlInterfaceProbes")
			<< "Cannot select probe with index " << index << ". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
		mitkThrow() << "Cannot select probe with index " << index <<
			". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
	}

	m_SelectedProbeIndex = index;
}

void mitk::USDiPhASProbesControls::OnSelectProbe(mitk::USProbe::Pointer probe)
{
}

mitk::USProbe::Pointer mitk::USDiPhASProbesControls::GetSelectedProbe()
{
	if (m_SelectedProbeIndex >= m_ProbesSet.size())
	{
		MITK_ERROR("USDiPhASProbesControls")("USControlInterfaceProbes")
			<< "Cannot get active probe as the current index is" << m_SelectedProbeIndex <<
			". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
		mitkThrow() << "Cannot get active probe as the current index is" << m_SelectedProbeIndex <<
			". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
	}

	return m_ProbesSet.at(m_SelectedProbeIndex).GetPointer();
}

unsigned int mitk::USDiPhASProbesControls::GetProbesCount() const
{
	return m_ProbesSet.size();
}


void mitk::USDiPhASProbesControls::ProbeRemoved(unsigned int index)
{
	MITK_INFO << "Probe removed...";

	if (m_ProbesSet.size() > index)
	{
		m_ProbesSet.erase(m_ProbesSet.begin() + index);
	}
}

void mitk::USDiPhASProbesControls::ProbeAdded(unsigned int index)
{
	MITK_INFO << "Probe arrived...";

	this->CreateProbesSet();
}

void mitk::USDiPhASProbesControls::CreateProbesSet()
{
	m_ProbesSet.push_back(mitk::USDiPhASProbe::New( m_DiPhASDevice->GetScanMode().transducerName ));
}