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

#include "mitkUSFraunhoferProbesControls.h"
#include "mitkUSFraunhoferDevice.h"
#include <mitkException.h>

mitk::USFraunhoferProbesControls::USFraunhoferProbesControls(itk::SmartPointer<USFraunhoferDevice> device)
  : mitk::USControlInterfaceProbes(device.GetPointer()),
    m_IsActive(false), m_FraunhoferDevice(device)
{
}

mitk::USFraunhoferProbesControls::~USFraunhoferProbesControls()
{
}


void mitk::USFraunhoferProbesControls::SetIsActive(bool isActive)
{
	m_IsActive = isActive;
}

bool mitk::USFraunhoferProbesControls::GetIsActive()
{
	return m_IsActive;
}

std::vector<mitk::USProbe::Pointer> mitk::USFraunhoferProbesControls::GetProbeSet()
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

void mitk::USFraunhoferProbesControls::OnSelectProbe(unsigned int index)
{
	if (index >= m_ProbesSet.size())
	{
		MITK_ERROR("USFraunhoferProbesControls")("USControlInterfaceProbes")
			<< "Cannot select probe with index " << index << ". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
		mitkThrow() << "Cannot select probe with index " << index <<
			". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
	}

	m_SelectedProbeIndex = index;
}

void mitk::USFraunhoferProbesControls::OnSelectProbe(mitk::USProbe::Pointer probe)
{
}

mitk::USProbe::Pointer mitk::USFraunhoferProbesControls::GetSelectedProbe()
{
	if (m_SelectedProbeIndex >= m_ProbesSet.size())
	{
		MITK_ERROR("USFraunhoferProbesControls")("USControlInterfaceProbes")
			<< "Cannot get active probe as the current index is" << m_SelectedProbeIndex <<
			". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
		mitkThrow() << "Cannot get active probe as the current index is" << m_SelectedProbeIndex <<
			". Maximum possible index is " << m_ProbesSet.size() - 1 << ".";
	}

	return m_ProbesSet.at(m_SelectedProbeIndex).GetPointer();
}

unsigned int mitk::USFraunhoferProbesControls::GetProbesCount() const
{
	return m_ProbesSet.size();
}


void mitk::USFraunhoferProbesControls::ProbeRemoved(unsigned int index)
{
	MITK_INFO << "Probe removed...";

	if (m_ProbesSet.size() > index)
	{
		m_ProbesSet.erase(m_ProbesSet.begin() + index);
	}
}

void mitk::USFraunhoferProbesControls::ProbeAdded(unsigned int index)
{
	MITK_INFO << "Probe arrived...";

	this->CreateProbesSet();
}

void mitk::USFraunhoferProbesControls::CreateProbesSet()
{
	m_ProbesSet.push_back(mitk::USFraunhoferProbe::New( m_FraunhoferDevice->GetScanMode().transducerName ));
}