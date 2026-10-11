/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMxNDemoView.h"

#include <QmitkAbstractMultiWidgetEditor.h>

#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>
#include <berryUIException.h>

#include <mitkCoreServices.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkException.h>
#include <mitkIDataStorageService.h>

#include <ctkPlugin.h>

#include <nlohmann/json.hpp>

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

const std::string QmitkMxNDemoView::VIEW_ID = "org.mitk.example.views.mxndemo";
const QString QmitkMxNDemoView::EDITOR_ID = "org.mitk.example.editors.mxndemo";

namespace
{
  const QString PLUGIN_ID = QStringLiteral("org.mitk.example.gui.mxneditor");

  /** Reads a layout embedded in this plugin, the way the editor reads the one
   *  its declaration names. */
  nlohmann::json LoadLayoutResource(const QString& path)
  {
    const auto plugin = berry::Platform::GetPlugin(PLUGIN_ID);
    const auto content = nullptr != plugin ? plugin->getResource(path) : QByteArray();
    if (content.isEmpty())
    {
      mitkThrow() << "Layout resource '" << path.toStdString() << "' is missing.";
    }
    return nlohmann::json::parse(content.cbegin(), content.cend());
  }
}

QmitkMxNDemoView::~QmitkMxNDemoView()
{
  this->GetSite()->GetPage()->RemovePartListener(this);
}

berry::IPartListener::Events::Types QmitkMxNDemoView::GetPartEventTypes() const
{
  return Events::OPENED | Events::CLOSED;
}

void QmitkMxNDemoView::PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == EDITOR_ID)
  {
    this->SetMultiWidget(this->FindMultiWidget());
  }
}

void QmitkMxNDemoView::PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef)
{
  if (partRef->GetId() == EDITOR_ID)
  {
    this->SetMultiWidget(nullptr);
  }
}

void QmitkMxNDemoView::CreateQtPartControl(QWidget* parent)
{
  auto* layout = new QVBoxLayout(parent);

  m_OpenButton = new QPushButton(tr("Open demo display"), parent);
  layout->addWidget(m_OpenButton);
  connect(m_OpenButton, &QPushButton::clicked, this, &QmitkMxNDemoView::OpenDemoDisplay);

  const std::pair<QmitkMxNLayoutLock, QString> lockLabels[] = {
    { QmitkMxNLayoutLock::Structure, tr("Lock structure") },
    { QmitkMxNLayoutLock::Direction, tr("Lock view directions") },
    { QmitkMxNLayoutLock::Proportions, tr("Lock proportions") },
    { QmitkMxNLayoutLock::Content, tr("Lock content") }
  };
  for (const auto& [lock, label] : lockLabels)
  {
    auto* box = new QCheckBox(label, parent);
    layout->addWidget(box);
    connect(box, &QCheckBox::toggled, this, &QmitkMxNDemoView::ApplyLocksFromBoxes);
    m_LockBoxes[lock] = box;
  }

  m_SwitchButton = new QPushButton(tr("Switch layout"), parent);
  layout->addWidget(m_SwitchButton);
  connect(m_SwitchButton, &QPushButton::clicked, this, &QmitkMxNDemoView::SwitchLayout);

  m_Status = new QLabel(parent);
  m_Status->setWordWrap(true);
  layout->addWidget(m_Status);
  layout->addStretch();

  this->GetSite()->GetPage()->AddPartListener(this);
  this->SetMultiWidget(this->FindMultiWidget());
}

void QmitkMxNDemoView::SetFocus()
{
  m_OpenButton->setFocus();
}

void QmitkMxNDemoView::OpenDemoDisplay()
{
  mitk::CoreServicePointer<mitk::IDataStorageService> storageService(mitk::CoreServices::GetDataStorageService());
  if (!storageService)
  {
    return;
  }
  berry::IEditorInput::Pointer input(new mitk::DataStorageEditorInput(storageService->GetActiveDataStorageReference()));

  berry::IEditorPart::Pointer editor;
  try
  {
    // MATCH_ID: the data storage alone would also match the stock MxN editor.
    editor = this->GetSite()->GetPage()->OpenEditor(input, EDITOR_ID, true, berry::IWorkbenchPage::MATCH_ID);
  }
  catch (const berry::PartInitException& e)
  {
    m_Status->setText(tr("The demo display could not be opened: %1").arg(QString::fromUtf8(e.what())));
    return;
  }

  // The Qt workbench returns no editor, rather than throwing, when the part
  // fails to create.
  if (editor.IsNull())
  {
    m_Status->setText(tr("The demo display could not be opened; see the log."));
    return;
  }
  m_Status->clear();
  this->SetMultiWidget(this->FindMultiWidget());
}

void QmitkMxNDemoView::SwitchLayout()
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  const auto path = m_ShowsAltLayout
    ? QStringLiteral("resources/MxNDemoLayout.json")
    : QStringLiteral("resources/MxNDemoLayoutAlt.json");
  try
  {
    // Locks never block code: this works even with 'structure' locked.
    m_MultiWidget->ApplyLayout(LoadLayoutResource(path));
    m_ShowsAltLayout = !m_ShowsAltLayout;
    m_Status->clear();
  }
  catch (const QmitkMxNLayoutBusyException&)
  {
    m_Status->setText(tr("The display is still applying a layout; try again."));
  }
  catch (const mitk::Exception& e)
  {
    m_Status->setText(tr("The layout could not be applied: %1").arg(QString::fromStdString(e.GetDescription())));
  }
  catch (const nlohmann::json::exception& e)
  {
    m_Status->setText(tr("The layout could not be read: %1").arg(QString::fromUtf8(e.what())));
  }
}

void QmitkMxNDemoView::SetMultiWidget(QmitkMxNMultiWidget* multiWidget)
{
  QObject::disconnect(m_LocksConnection);

  // A new widget starts on the layout its declaration names.
  if (multiWidget != m_MultiWidget.data())
  {
    m_ShowsAltLayout = false;
  }
  m_MultiWidget = multiWidget;

  // Follows lock changes from anywhere, not only from this view's boxes.
  if (nullptr != multiWidget)
  {
    m_LocksConnection = connect(multiWidget, &QmitkMxNMultiWidget::LayoutLocksChanged,
                                this, &QmitkMxNDemoView::ShowLocks);
  }

  for (const auto& [lock, box] : m_LockBoxes)
  {
    box->setEnabled(nullptr != multiWidget);
  }
  m_SwitchButton->setEnabled(nullptr != multiWidget);
  this->ShowLocks(nullptr != multiWidget ? multiWidget->GetLayoutLocks() : QmitkMxNLayoutLocks());
}

QmitkMxNMultiWidget* QmitkMxNDemoView::FindMultiWidget() const
{
  const auto editors = this->GetSite()->GetPage()->FindEditors(
    berry::IEditorInput::Pointer(), EDITOR_ID, berry::IWorkbenchPage::MATCH_ID);
  for (const auto& reference : editors)
  {
    // An editor showing its error panel has no multi widget.
    auto* editor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(reference->GetEditor(false).GetPointer());
    auto* multiWidget = nullptr != editor ? dynamic_cast<QmitkMxNMultiWidget*>(editor->GetMultiWidget()) : nullptr;
    if (nullptr != multiWidget)
    {
      return multiWidget;
    }
  }
  return nullptr;
}

void QmitkMxNDemoView::ShowLocks(QmitkMxNLayoutLocks locks)
{
  for (const auto& [lock, box] : m_LockBoxes)
  {
    const QSignalBlocker blocker(box);
    box->setChecked(locks.testFlag(lock));
  }
}

void QmitkMxNDemoView::ApplyLocksFromBoxes()
{
  if (m_MultiWidget.isNull())
  {
    return;
  }

  QmitkMxNLayoutLocks locks;
  for (const auto& [lock, box] : m_LockBoxes)
  {
    locks.setFlag(lock, box->isChecked());
  }
  m_MultiWidget->SetLayoutLocks(locks);
}
