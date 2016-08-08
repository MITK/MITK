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
	std::vector<mitk::USProbe::Pointer> usProbes(1, 0);

	return usProbes;
}

void mitk::USFraunhoferProbesControls::OnSelectProbe(unsigned int index)
{

}

void mitk::USFraunhoferProbesControls::OnSelectProbe(mitk::USProbe::Pointer probe)
{
}

mitk::USProbe::Pointer mitk::USFraunhoferProbesControls::GetSelectedProbe()
{

	return nullptr;
}

unsigned int mitk::USFraunhoferProbesControls::GetProbesCount() const
{
	return 0;
}


void mitk::USFraunhoferProbesControls::ProbeRemoved(unsigned int index)
{
	MITK_INFO << "Probe removed...";
}

void mitk::USFraunhoferProbesControls::ProbeAdded(unsigned int index)
{
	MITK_INFO << "Probe arrived...";
}

bool mitk::USFraunhoferProbesControls::CreateProbesCollection()
{
	return true;
}

void mitk::USFraunhoferProbesControls::CreateProbesSet()
{
	
}