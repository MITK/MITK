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

// MITK
#include "mitkUSDeviceWriterXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>
#include <mitkUSDevice.h>

// Third Party
#include <tinyxml.h>
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <iostream>

mitk::USDeviceWriterXML::USDeviceWriterXML() : AbstractFileWriter(USDevice::GetStaticNameOfClass(),
  mitk::IGTMimeTypes::USDEVICEINFORMATIONXML_MIMETYPE(),
  "MITK USDevice Writer (XML)"), m_Filename("")
{
  RegisterService();
}

mitk::USDeviceWriterXML::USDeviceWriterXML(const mitk::USDeviceWriterXML& other) : AbstractFileWriter(other)
{
}

mitk::USDeviceWriterXML::~USDeviceWriterXML()
{
}

mitk::USDeviceWriterXML* mitk::USDeviceWriterXML::Clone() const
{
  return new USDeviceWriterXML(*this);
}

void mitk::USDeviceWriterXML::Write()
{
  if (m_Filename == "")
  {
    MITK_WARN << "Cannot write to file - empty filename!";
    return;
  }
}

void mitk::USDeviceWriterXML::SetFilename(std::string filename)
{
  m_Filename = filename;
}
