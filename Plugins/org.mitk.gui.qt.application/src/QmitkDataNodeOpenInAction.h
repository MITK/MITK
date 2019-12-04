/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKDATANODEOPENINACTION_H
#define QMITKDATANODEOPENINACTION_H

#include <org_mitk_gui_qt_application_Export.h>

#include "QmitkAbstractDataNodeAction.h"

// mitk core
#include <mitkBaseRenderer.h>

// qt
#include <QAction>

class MITK_QT_APP QmitkDataNodeOpenInAction : public QAction, public QmitkAbstractDataNodeAction
{
  Q_OBJECT

public:

  typedef std::vector<mitk::BaseRenderer*> RendererVector;

  QmitkDataNodeOpenInAction(QWidget* parent, berry::IWorkbenchPartSite::Pointer workbenchPartSite);
  QmitkDataNodeOpenInAction(QWidget* parent, berry::IWorkbenchPartSite* workbenchPartSite);

  void SetControlledRenderer(RendererVector controlledRenderer);

private Q_SLOTS:

  void OnMenuAboutToShow();
  void OnActionTriggered(bool);

protected:

  void InitializeAction() override;

  void SetControlledRenderer();

  RendererVector m_ControlledRenderer;

};

#endif // QMITKDATANODEOPENINACTION_H
