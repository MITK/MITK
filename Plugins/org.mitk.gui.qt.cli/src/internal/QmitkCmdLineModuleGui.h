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

#ifndef QmitkCmdLineModuleGui_h
#define QmitkCmdLineModuleGui_h

#include <QBuffer>
#include <QUiLoader>

#include <ctkCmdLineModuleQtGui_p.h>
#include <ctkCmdLineModuleReference.h>
#include "mitkDataStorage.h"

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
