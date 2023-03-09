/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTBaseActivator_h
#define mitkIGTBaseActivator_h

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

#endif
