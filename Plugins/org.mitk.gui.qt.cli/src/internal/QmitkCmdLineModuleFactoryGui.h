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

#ifndef QmitkCmdLineModuleFactoryGui_h
#define QmitkCmdLineModuleFactoryGui_h

#include <ctkCmdLineModuleFactory.h>
#include "mitkDataStorage.h"

class QmitkCmdLineModuleFactoryGui
    : public ctkCmdLineModuleFactory
{
public:

  QmitkCmdLineModuleFactoryGui(const mitk::DataStorage* dataStorage);
  virtual ~QmitkCmdLineModuleFactoryGui();

  ctkCmdLineModule* create(const ctkCmdLineModuleReference& moduleRef);

private:

  const mitk::DataStorage* m_DataStorage;

};

#endif // QmitkCmdLineModuleFactoryGui_h
