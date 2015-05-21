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

#ifndef MITKIGTBASEACTIVATOR_H
#define MITKIGTBASEACTIVATOR_H

#include <usModuleActivator.h>

#include <memory>

namespace mitk {

struct IFileReader;
struct IFileWriter;

class IOExtActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext*context) override;
  void Unload(us::ModuleContext* context) override;

private:

  std::unique_ptr<IFileWriter> m_NavigationDataSetWriterXML;
  std::unique_ptr<IFileWriter> m_NavigationDataSetWriterCSV;
  std::unique_ptr<IFileReader> m_NavigationDataReaderXML;
  std::unique_ptr<IFileReader> m_NavigationDataReaderCSV;
};

}

#endif // MITKIGTBASEACTIVATOR_H
