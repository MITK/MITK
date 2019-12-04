/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTBaseActivator.h"

#include <mitkNavigationDataSetWriterXML.h>
#include <mitkNavigationDataSetWriterCSV.h>
#include <mitkNavigationDataReaderCSV.h>
#include <mitkNavigationDataReaderXML.h>

namespace mitk {

void IOExtActivator::Load(us::ModuleContext*)
{
  m_NavigationDataSetWriterXML.reset(new NavigationDataSetWriterXML());
  m_NavigationDataSetWriterCSV.reset(new NavigationDataSetWriterCSV());
  m_NavigationDataReaderCSV.reset(new NavigationDataReaderCSV());
  m_NavigationDataReaderXML.reset(new NavigationDataReaderXML());

}

void IOExtActivator::Unload(us::ModuleContext*)
{
}

}

US_EXPORT_MODULE_ACTIVATOR(mitk::IOExtActivator)
