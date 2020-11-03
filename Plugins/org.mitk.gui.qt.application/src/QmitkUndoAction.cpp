/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUndoAction.h"

#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <mitkWorkbenchUtil.h>

#include <mitkUndoController.h>
#include <mitkVerboseLimitedLinearUndo.h>

#include <berryIPreferences.h>

class QmitkUndoActionPrivate
{
public:

  void init ( berry::IWorkbenchWindow* window, QmitkUndoAction* action )
  {
    m_Window = window;
    action->setText("&Undo");
    action->setToolTip("Undo the last action (not supported by all modules)");

    QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(Run()));
  }

  berry::IWorkbenchWindow* m_Window;
};

QmitkUndoAction::QmitkUndoAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr), d(new QmitkUndoActionPrivate)
{
  d->init(window.GetPointer(), this);
}

QmitkUndoAction::QmitkUndoAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr), d(new QmitkUndoActionPrivate)
{
  d->init(window.GetPointer(), this);
  this->setIcon(icon);
}

QmitkUndoAction::QmitkUndoAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(nullptr), d(new QmitkUndoActionPrivate)
{
  d->init(window, this);
  this->setIcon(icon);
}

QmitkUndoAction::~QmitkUndoAction()
{
}

void QmitkUndoAction::Run()
{
  mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
  if (model)
  {
    if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
    {
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions =
        verboseundo->GetUndoDescriptions();
      if (descriptions.size() >= 1)
      {
        MITK_INFO << "Undo " << descriptions.front().second;
      }
    }
    model->Undo();
  }
  else
  {
    MITK_ERROR << "No undo model instantiated";
  }
}
