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

#include "mitkUSCombinedModality.h"
#include "mitkUSDevice.h"
#include "mitkNavigationDataSource.h"

const std::string mitk::USCombinedModality::DeviceClassIdentifier = "org.mitk.modules.us.USCombinedModality";

mitk::USCombinedModality::USCombinedModality(std::string manufacturer, std::string model)
    : mitk::USDevice(manufacturer, model)
{

}

mitk::USCombinedModality::~USCombinedModality()
{

}


std::string mitk::USCombinedModality::GetDeviceClass()
{
    return DeviceClassIdentifier;
}

mitk::USImageSource::Pointer mitk::USCombinedModality::GetUSImageSource()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->GetUSImageSource();
}

mitk::USControlInterfaceBMode::Pointer mitk::USCombinedModality::GetControlInterfaceBMode()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->GetControlInterfaceBMode();
}

mitk::USControlInterfaceProbes::Pointer mitk::USCombinedModality::GetControlInterfaceProbes()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->GetControlInterfaceProbes();
}

mitk::USControlInterfaceDoppler::Pointer mitk::USCombinedModality::GetControlInterfaceDoppler()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->GetControlInterfaceDoppler();
}

bool mitk::USCombinedModality::OnInitialization()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->Initialize();
}

bool mitk::USCombinedModality::OnConnection()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->Connect();
}

bool mitk::USCombinedModality::OnDisconnection()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->Disconnect();
}

bool mitk::USCombinedModality::OnActivation()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    return m_UltrasoundDevice->Activate();
}

bool mitk::USCombinedModality::OnDeactivation()
{
    if (m_UltrasoundDevice.IsNull())
    {
        MITK_ERROR("USCombinedModality")("USDevice") << "UltrasoundDevice must not be null.";
        mitkThrow() << "UltrasoundDevice must not be null.";
    }

    m_UltrasoundDevice->Deactivate();

    return m_UltrasoundDevice->GetIsInitialized();
}

void mitk::USCombinedModality::GenerateData()
{
}
