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

#ifndef QMITKLESIONSHOWINACTION_H
#define QMITKLESIONSHOWINACTION_H

// mitk gui qt application plugin
#include <QmitkAbstractDataNodeAction.h>

// semantic relations module
#include <mitkSemanticTypes.h>

// qt
#include <QAction>

class QmitkFocusOnLesionAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  typedef std::vector<mitk::BaseRenderer*> RendererVector;

  QmitkFocusOnLesionAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkFocusOnLesionAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  void SetSelectedLesion(mitk::SemanticTypes::Lesion selectedLesion);

private Q_SLOTS:

  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  void SetControlledRenderer();

  RendererVector m_ControlledRenderer;
  mitk::SemanticTypes::Lesion m_Lesion;
};

#endif // QMITKLESIONSHOWINACTION_H
