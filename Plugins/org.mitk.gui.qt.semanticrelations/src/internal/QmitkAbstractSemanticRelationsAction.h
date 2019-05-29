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

#ifndef QMITKABSTRACTSEMANTICRELATIONSACTION_H
#define QMITKABSTRACTSEMANTICRELATIONSACTION_H

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// semantic relations module
#include <mitkSemanticRelationsIntegration.h>

class QmitkAbstractSemanticRelationsAction : public QmitkAbstractDataNodeAction
{

public:

  QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite* workbenchPartSite);

protected:

  std::unique_ptr<mitk::SemanticRelationsIntegration> m_SemanticRelationsIntegration;
};

#endif // QMITKABSTRACTSEMANTICRELATIONSACTION_H
