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

#include <QmitkIOUtil.h>

#include <QFileDialog>
#include <QMessageBox>

class QmitkFileSaveActionPrivate
{
private:

  void HandleSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, berry::ISelection::ConstPointer selection)
  {
    this->setEnabled(selection);
  }

  berry::ISelectionListener::Pointer m_SelectionListener;

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
      m_Window.Lock()->GetSelectionService()->RemoveSelectionListener(m_SelectionListener);
    }
  }

  void init ( berry::IWorkbenchWindow::Pointer window, QmitkFileSaveAction* action )
  {
    m_Window = window;
    m_Action = action;

    action->setParent(static_cast<QWidget*>(m_Window.Lock()->GetShell()->GetControl()));
    action->setText("&Save...");
    action->setToolTip("Save data objects (images, surfaces,...)");

    berry::ISelectionService* selectionService = m_Window.Lock()->GetSelectionService();
    setEnabled(selectionService->GetSelection());

    selectionService->AddSelectionListener(m_SelectionListener);

    QObject::connect(action, SIGNAL(triggered(bool)), action, SLOT(Run()));
  }

  berry::IPreferences::Pointer GetPreferences() const
  {
    berry::IPreferencesService::Pointer prefService = mitk::PluginActivator::GetInstance()->GetPreferencesService();
    if (prefService.IsNotNull())
    {
      return prefService->GetSystemPreferences()->Node("/General");
    }
    return berry::IPreferences::Pointer(0);
  }

  QString getLastFileSavePath() const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      return QString::fromStdString(prefs->Get("LastFileSavePath", ""));
    }
    return QString();
  }

  void setLastFileSavePath(const QString& path) const
  {
    berry::IPreferences::Pointer prefs = GetPreferences();
    if(prefs.IsNotNull())
    {
      prefs->Put("LastFileSavePath", path.toStdString());
      prefs->Flush();
    }
  }

  void setEnabled(berry::ISelection::ConstPointer selection)
  {
    mitk::DataNodeSelection::ConstPointer nodeSelection = selection.Cast<const mitk::DataNodeSelection>();
    if (nodeSelection.IsNotNull() && !selection->IsEmpty())
    {
      bool enable = false;
      std::list<mitk::DataNode::Pointer> dataNodes = nodeSelection->GetSelectedDataNodes();
      for (std::list<mitk::DataNode::Pointer>::const_iterator nodeIter = dataNodes.begin(),
           nodeIterEnd = dataNodes.end(); nodeIter != nodeIterEnd; ++nodeIter)
      {
        if ((*nodeIter)->GetData() != NULL)
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
  : QAction(0), d(new QmitkFileSaveActionPrivate)
{
  d->init(window, this);
}

QmitkFileSaveAction::QmitkFileSaveAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(0), d(new QmitkFileSaveActionPrivate)
{
  d->init(window, this);
  this->setIcon(icon);
}

QmitkFileSaveAction::~QmitkFileSaveAction()
{
}

void QmitkFileSaveAction::Run()
{
  // Get the list of selected base data objects
  mitk::DataNodeSelection::ConstPointer selection = d->m_Window.Lock()->GetSelectionService()->GetSelection().Cast<const mitk::DataNodeSelection>();
  if (selection.IsNull() || selection->IsEmpty())
  {
    MITK_ERROR << "Assertion failed: data node selection is NULL or empty";
    return;
  }

  std::list<mitk::DataNode::Pointer> dataNodes = selection->GetSelectedDataNodes();

  std::vector<const mitk::BaseData*> data;
  QStringList names;
  for (std::list<mitk::DataNode::Pointer>::const_iterator nodeIter = dataNodes.begin(),
       nodeIterEnd = dataNodes.end(); nodeIter != nodeIterEnd; ++nodeIter)
  {
    data.push_back((*nodeIter)->GetData());
    std::string name;
    (*nodeIter)->GetStringProperty("name", name);
    names.push_back(QString::fromStdString(name));
  }

  try
  {
    QStringList fileNames = QmitkIOUtil::Save(data, names, d->getLastFileSavePath(),
                                              d->m_Action->parentWidget());
    if (!fileNames.empty())
    {
      d->setLastFileSavePath(QFileInfo(fileNames.back()).absolutePath());
    }
  }
  catch (const mitk::Exception& e)
  {
    MITK_INFO << e;
    return;
  }
}
