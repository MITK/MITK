/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
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

/**
 * \class QmitkCmdLineModuleFactoryGui
 * \brief Derived from ctkCmdLineModuleFactory to instantiate QmitkCmdLineModuleGui
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cli_internal
 * \sa QmitkCmdLineModuleGui
 */
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
