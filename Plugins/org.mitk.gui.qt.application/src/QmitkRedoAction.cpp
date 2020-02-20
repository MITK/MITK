/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRedoAction.h"

#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <mitkWorkbenchUtil.h>

#include <mitkUndoController.h>
#include <mitkVerboseLimitedLinearUndo.h>

#include <berryIPreferences.h>

class QmitkRedoActionPrivate
{
public:

  void init ( berry::IWorkbenchWindow* window, QmitkRedoAction* action )
  {
    m_Window = window;
    action->setText("&Redo");
    action->setToolTip("execute the last action that was undone again (not supported by all modules)");

    QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(Run()));
  }

  berry::IWorkbenchWindow* m_Window;
};

QmitkRedoAction::QmitkRedoAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr), d(new QmitkRedoActionPrivate)
{
  d->init(window.GetPointer(), this);
}

QmitkRedoAction::QmitkRedoAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr), d(new QmitkRedoActionPrivate)
{
  d->init(window.GetPointer(), this);
  this->setIcon(icon);
}

QmitkRedoAction::QmitkRedoAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(nullptr), d(new QmitkRedoActionPrivate)
{
  d->init(window, this);
  this->setIcon(icon);
}

QmitkRedoAction::~QmitkRedoAction()
{
}

void QmitkRedoAction::Run()
{
  mitk::UndoModel* model = mitk::UndoController::GetCurrentUndoModel();
  if (model)
  {
    if (mitk::VerboseLimitedLinearUndo* verboseundo = dynamic_cast<mitk::VerboseLimitedLinearUndo*>( model ))
    {
      mitk::VerboseLimitedLinearUndo::StackDescription descriptions =
        verboseundo->GetRedoDescriptions();
      if (descriptions.size() >= 1)
      {
        MITK_INFO << "Redo " << descriptions.front().second;
      }
    }
    model->Redo();
  }
  else
  {
    MITK_ERROR << "No undo model instantiated";
  }
}
