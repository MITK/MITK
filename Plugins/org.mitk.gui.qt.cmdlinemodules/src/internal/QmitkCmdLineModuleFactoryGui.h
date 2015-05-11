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

#include <ctkCmdLineModuleFrontendFactoryQtGui.h>
#include <ctkCmdLineModuleReference.h>

namespace mitk {
  class DataStorage;
}
struct QmitkCmdLineModuleFactoryGuiPrivate;

/**
 * \class QmitkCmdLineModuleFactoryGui
 * \brief Derived from ctkCmdLineModuleFactory to instantiate QmitkCmdLineModuleGui front ends.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \sa QmitkCmdLineModuleGui
 * \sa ctkCmdLineModuleFrontendFactoryQtGui
 */
class QmitkCmdLineModuleFactoryGui
    : public ctkCmdLineModuleFrontendFactoryQtGui
{
public:

  QmitkCmdLineModuleFactoryGui(const mitk::DataStorage* dataStorage);
  virtual ~QmitkCmdLineModuleFactoryGui();

  /**
   * \brief Simply creates QmitkCmdLineModuleGui which is an MITK specific Qt front end.
   */
  ctkCmdLineModuleFrontendQtGui* create(const ctkCmdLineModuleReference& moduleRef) override;

private:

  QScopedPointer<QmitkCmdLineModuleFactoryGuiPrivate> d;

};

#endif // QmitkCmdLineModuleFactoryGui_h
