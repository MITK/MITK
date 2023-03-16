/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFocusOnLesionAction_h
#define QmitkFocusOnLesionAction_h

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

  void InitializeAction() override;

  void SetControlledRenderer();

  RendererVector m_ControlledRenderer;
  mitk::SemanticTypes::Lesion m_Lesion;
};

#endif
