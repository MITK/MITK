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

#include "mitkUSVideoDeviceCustomControls.h"

mitk::USVideoDeviceCustomControls::USVideoDeviceCustomControls(itk::SmartPointer<USDevice> device)
  : mitk::USAbstractControlInterface(device.GetPointer()), m_IsActive(false)
{
  m_ImageSource = dynamic_cast<mitk::USImageVideoSource*>(m_Device->GetUSImageSource().GetPointer());
}

mitk::USVideoDeviceCustomControls::~USVideoDeviceCustomControls()
{
}

void mitk::USVideoDeviceCustomControls::SetIsActive(bool isActive)
{
  m_IsActive = isActive;
}

bool mitk::USVideoDeviceCustomControls::GetIsActive()
{
  return m_IsActive;
}

void mitk::USVideoDeviceCustomControls::SetCropArea(mitk::USImageVideoSource::USImageCropping newArea)
{
  MITK_INFO << "Set Crop Area L:" << newArea.left << " R:" << newArea.right
    << " T:" << newArea.top << " B:" << newArea.bottom;

  if (m_ImageSource.IsNotNull())
  {
    // if area is empty, remove region
    if ((newArea.bottom == 0) && (newArea.top == 0) &&
      (newArea.left == 0) && (newArea.right == 0))
    {
      m_ImageSource->RemoveRegionOfInterest();
    }
    else
    {
      m_ImageSource->SetCropping(newArea);
    }
  }
  else
  {
    MITK_WARN << "Cannot set crop are, source is not initialized!";
  }
}

void mitk::USVideoDeviceCustomControls::SetNewDepth(double depth)
{
  if (m_Device.IsNotNull())
  {
    if( m_Device->GetCurrentProbe().IsNotNull() )
    {
      m_Device->GetCurrentProbe()->SetCurrentDepth(depth);
      MITK_INFO << "SetCurrentDepth of currentProbe: " << depth;
    }
  }
  m_Device->DepthChanged(depth);
}

void mitk::USVideoDeviceCustomControls::SetNewProbeIdentifier(std::string probename)
{
  if( m_Device.IsNotNull() )
  {
    m_Device->SetCurrentProbe(probename);
  }
  m_Device->ProbeChanged(probename);
}

mitk::USProbe::USProbeCropping mitk::USVideoDeviceCustomControls::GetCropArea()
{
  // just return the crop area set at the image source
  mitk::USVideoDevice::Pointer device = dynamic_cast<mitk::USVideoDevice*>(m_Device.GetPointer());
  if (device.IsNotNull())
  {
    mitk::USProbe::Pointer probe = device->GetCurrentProbe();
    if (probe.IsNotNull())
    {
      return probe->GetProbeCropping();
    }
  }
  mitk::USProbe::USProbeCropping defaultCropping;
  return defaultCropping;
}

void mitk::USVideoDeviceCustomControls::UpdateProbeCropping(mitk::USImageVideoSource::USImageCropping cropping)
{
  mitk::USVideoDevice::Pointer device = dynamic_cast<mitk::USVideoDevice*>(m_Device.GetPointer());
  if (device.IsNotNull())
  {
    mitk::USProbe::Pointer probe = device->GetCurrentProbe();
    if( probe.IsNotNull() )
    {
      probe->SetProbeCropping(cropping.top, cropping.bottom, cropping.left, cropping.right);
    }
  }
}

std::vector<mitk::USProbe::Pointer> mitk::USVideoDeviceCustomControls::GetProbes()
{
  return m_Device->GetAllProbes();
}

std::vector<int> mitk::USVideoDeviceCustomControls::GetDepthsForProbe(std::string name)
{
  mitk::USProbe::Pointer probe = m_Device->GetProbeByName(name);
  std::map<int, mitk::Vector3D> depthsAndSpacings = probe->GetDepthsAndSpacing();
  std::vector<int> depths;
  for (std::map<int, mitk::Vector3D>::iterator it = depthsAndSpacings.begin(); it != depthsAndSpacings.end(); it++)
  {
    depths.push_back((it->first));
  }
  return depths;
}

void mitk::USVideoDeviceCustomControls::SetDefaultProbeAsCurrentProbe()
{
  m_Device->SetDefaultProbeAsCurrentProbe();
}
