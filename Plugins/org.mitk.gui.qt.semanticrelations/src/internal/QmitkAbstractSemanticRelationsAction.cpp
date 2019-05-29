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

// semantic relations plugin
#include "QmitkAbstractSemanticRelationsAction.h"

QmitkAbstractSemanticRelationsAction::QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite::Pointer workbenchPartSite)
  : QmitkAbstractDataNodeAction(workbenchPartSite)
{
  m_SemanticRelationsIntegration = std::make_unique<mitk::SemanticRelationsIntegration>();
}

QmitkAbstractSemanticRelationsAction::QmitkAbstractSemanticRelationsAction(berry::IWorkbenchPartSite* workbenchPartSite)
  : QmitkAbstractDataNodeAction(berry::IWorkbenchPartSite::Pointer(workbenchPartSite))
{
  // nothing here
}
