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

  virtual ~QmitkDataNodeOpenInAction() override;

  void SetControlledRenderer(RendererVector controlledRenderer);

private Q_SLOTS:

  void OnMenuAboutToShow();
  void OnActionTriggered(bool);

protected:

  virtual void InitializeAction() override;

  void SetControlledRenderer();

  RendererVector m_ControlledRenderer;

};

#endif // QMITKDATANODEOPENINACTION_H
