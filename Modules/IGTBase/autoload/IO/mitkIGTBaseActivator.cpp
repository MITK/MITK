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

#include "mitkIGTBaseActivator.h"

#include "mitkNavigationDataSetWriterXML.h"
#include "mitkNavigationDataSetWriterCSV.h"


namespace mitk {

void IOExtActivator::Load(us::ModuleContext*)
{
  m_NavigationDataSetWriterXML.reset(new NavigationDataSetWriterXML());
  m_NavigationDataSetWriterCSV.reset(new NavigationDataSetWriterCSV());
}

void IOExtActivator::Unload(us::ModuleContext*)
{
}

}

US_EXPORT_MODULE_ACTIVATOR(mitk::IOExtActivator)
