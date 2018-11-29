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

#include "QmitkFileSaveAction.h"

#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <mitkWorkbenchUtil.h>
#include <mitkDataNodeSelection.h>

#include <berryISelectionService.h>
#include <berryINullSelectionListener.h>
#include <berryIPreferences.h>

#include <QmitkIOUtil.h>

class QmitkFileSaveActionPrivate
{
private:

  void HandleSelectionChanged(const berry::IWorkbenchPart::Pointer& /*part*/,
                              const berry::ISelection::ConstPointer& selection)
  {
    this->SetEnabled(selection);
  }

  QScopedPointer<berry::ISelectionListener> m_SelectionListener;

public:

  QmitkFileSaveActionPrivate()
    : m_SelectionListener(new berry::NullSelectionChangedAdapter<QmitkFileSaveActionPrivate>(
                            this, &QmitkFileSaveActionPrivate::HandleSelectionChanged))
  {
  }

  ~QmitkFileSaveActionPrivate()
  {
    if (!m_Window.Expired())
    {
      m_Window.Lock()->GetSelectionService()->RemoveSelectionListener(m_SelectionListener.data());
    }
  }

  void Init(berry::IWorkbenchWindow* window, QAction* action)
  {
    m_Window = berry::IWorkbenchWindow::Pointer(window);
    m_Action = action;

    m_Action->setText("&Save...");
    m_Action->setToolTip("Save data objects (images, surfaces,...)");

    berry::ISelectionService* selectionService = m_Window.Lock()->GetSelectionService();
    SetEnabled(selectionService->GetSelection());

    selectionService->AddSelectionListener(m_SelectionListener.data());

    QObject::connect(m_Action, SIGNAL(triggered(bool)), m_Action, SLOT(Run()));
  }

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService* prefService = mitk::PluginActivator::GetInstance()->GetPreferencesService();
    if (prefService != nullptr)
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(nullptr);
  }

  QString GetLastFileSavePath() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
      return prefs->Get("LastFileSavePath", "");
    }
    return QString();
  }

  void SetLastFileSavePath(const QString& path) const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if (prefs.IsNotNull())
    {
      prefs->Put("LastFileSavePath", path);
      prefs->Flush();
    }
  }

  void SetEnabled(berry::ISelection::ConstPointer selection)
  {
    mitk::DataNodeSelection::ConstPointer nodeSelection = selection.Cast<const mitk::DataNodeSelection>();
    if (nodeSelection.IsNotNull() && !selection->IsEmpty())
    {
      bool enable = false;
      std::list<mitk::DataNode::Pointer> dataNodes = nodeSelection->GetSelectedDataNodes();
      for (std::list<mitk::DataNode::Pointer>::const_iterator nodeIter = dataNodes.begin(), nodeIterEnd = dataNodes.end(); nodeIter != nodeIterEnd; ++nodeIter)
      {
        if ((*nodeIter)->GetData() != nullptr)
        {
          enable = true;
          break;
        }
      }
      m_Action->setEnabled(enable);
    }
    else
    {
      m_Action->setEnabled(false);
    }
  }

  berry::IWorkbenchWindow::WeakPtr m_Window;
  QAction* m_Action;
};

QmitkFileSaveAction::QmitkFileSaveAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(tr("Save..."))
  , d(new QmitkFileSaveActionPrivate)
{
  d->Init(window.GetPointer(), this);
}

QmitkFileSaveAction::QmitkFileSaveAction(const QIcon& icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(tr("Save..."))
  , d(new QmitkFileSaveActionPrivate)
{
  d->Init(window.GetPointer(), this);
  setIcon(icon);
}

QmitkFileSaveAction::QmitkFileSaveAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(tr("Save..."))
  , d(new QmitkFileSaveActionPrivate)
{
  d->Init(window, this);
  setIcon(icon);
}

QmitkFileSaveAction::~QmitkFileSaveAction()
{
}

void QmitkFileSaveAction::Run()
{
  // get the list of selected base data objects
  mitk::DataNodeSelection::ConstPointer selection = d->m_Window.Lock()->GetSelectionService()->GetSelection().Cast<const mitk::DataNodeSelection>();
  if (selection.IsNull() || selection->IsEmpty())
  {
    MITK_ERROR << "Assertion failed: data node selection is nullptr or empty";
    return;
  }

  std::list<mitk::DataNode::Pointer> dataNodes = selection->GetSelectedDataNodes();

  std::vector<const mitk::BaseData*> data;
  QStringList names;
  for (std::list<mitk::DataNode::Pointer>::const_iterator nodeIter = dataNodes.begin(), nodeIterEnd = dataNodes.end(); nodeIter != nodeIterEnd; ++nodeIter)
  {
    data.push_back((*nodeIter)->GetData());
    std::string name;
    (*nodeIter)->GetStringProperty("name", name);
    names.push_back(QString::fromStdString(name));
  }

  try
  {
    QStringList fileNames = QmitkIOUtil::Save(data, names, d->GetLastFileSavePath(), d->m_Action->parentWidget());
    if (!fileNames.empty())
    {
      d->SetLastFileSavePath(QFileInfo(fileNames.back()).absolutePath());
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_INFO << e;
    return;
  }
}
