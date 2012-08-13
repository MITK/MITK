/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkCmdLineModuleGui_h
#define QmitkCmdLineModuleGui_h

#include <QBuffer>
#include <QUiLoader>

#include <ctkCmdLineModuleQtGui_p.h>
#include <ctkCmdLineModuleReference.h>
#include "mitkDataStorage.h"

/**
 * \class QmitkCmdLineModuleGui
 * \brief Derived from ctkCmdLineModuleQtGui to implement a Qt specific command line module.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cli_internal
 * \sa QmitkCmdLineModuleFactoryGui
 */
class QmitkCmdLineModuleGui : public ctkCmdLineModuleQtGui
{
  Q_OBJECT

public:
  QmitkCmdLineModuleGui(const mitk::DataStorage* dataStorage, const ctkCmdLineModuleReference& moduleRef);
  virtual ~QmitkCmdLineModuleGui();

protected:

  virtual QUiLoader* uiLoader() const;
  virtual ctkCmdLineModuleXslTransform* xslTransform() const;

private:

  const mitk::DataStorage* m_DataStorage;

}; // end class

#endif // QmitkCmdLineModuleGui_h
