/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  ~QmitkCmdLineModuleFactoryGui() override;

  /**
   * \brief Simply creates QmitkCmdLineModuleGui which is an MITK specific Qt front end.
   */
  ctkCmdLineModuleFrontendQtGui* create(const ctkCmdLineModuleReference& moduleRef) override;

private:

  QScopedPointer<QmitkCmdLineModuleFactoryGuiPrivate> d;

};

#endif
