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

mitk::USVideoDeviceCustomControls::USVideoDeviceCustomControls(itk::SmartPointer<USVideoDevice> device)
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
  m_Device->DepthChanged(depth);
}

void mitk::USVideoDeviceCustomControls::SetNewProbeIdentifier(std::string probename)
{
  m_Device->ProbeChanged(probename);
}

mitk::USImageVideoSource::USImageCropping mitk::USVideoDeviceCustomControls::GetCropArea()
{
  // just return the crop area set at the image source
  return m_ImageSource->GetCropping();
}

std::vector<mitk::USProbe::Pointer> mitk::USVideoDeviceCustomControls::GetProbes()
{
  mitk::USVideoDevice::Pointer device = dynamic_cast<mitk::USVideoDevice*>(m_Device.GetPointer());
  return device->GetAllProbes();
}

std::vector<int> mitk::USVideoDeviceCustomControls::GetDepthsForProbe(std::string name)
{
  mitk::USVideoDevice::Pointer device = dynamic_cast<mitk::USVideoDevice*>(m_Device.GetPointer());
  mitk::USProbe::Pointer probe = device->GetProbeByName(name);
  std::map<int, mitk::Vector3D> depthsAndSpacings = probe->GetDepthsAndSpacing();
  std::vector<int> depths;
  for (std::map<int, mitk::Vector3D>::iterator it = depthsAndSpacings.begin(); it != depthsAndSpacings.end(); it++)
  {
    depths.push_back((it->first));
  }
  return depths;
}
