/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "org_mitk_gui_qt_flow_segmentation_Activator.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryQtStyleManager.h>
#include <berryPlatformUI.h>

#include <itksys/SystemTools.hxx>

//MITK
#include <mitkBaseApplication.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkIOUtil.h>
#include <mitkToolManagerProvider.h>

// Qmitk
#include "QmitkSegmentationFlowControlView.h"

#include <ui_QmitkSegmentationFlowControlView.h>

// Qt
#include <QMessageBox>
#include <QDir>
#include <QShortcut>

const std::string QmitkSegmentationFlowControlView::VIEW_ID = "org.mitk.views.flow.control";

QmitkSegmentationFlowControlView::QmitkSegmentationFlowControlView()
    : m_Controls(new Ui::SegmentationFlowControlView),
      m_Parent(nullptr)
{
  auto notHelperObject = mitk::NodePredicateNot::New(
    mitk::NodePredicateProperty::New("helper object"));

  m_SegmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::LabelSetImage>::New(),
    notHelperObject);

  m_SegmentationTaskListPredicate = mitk::NodePredicateAnd::New(
    mitk::TNodePredicateDataType<mitk::SegmentationTaskList>::New(),
    notHelperObject);

  berry::PlatformUI::GetWorkbench()->AddWorkbenchListener(this);
}

QmitkSegmentationFlowControlView::~QmitkSegmentationFlowControlView()
{
  berry::PlatformUI::GetWorkbench()->RemoveWorkbenchListener(this);
}

bool QmitkSegmentationFlowControlView::PreShutdown(berry::IWorkbench*, bool)
{
  return m_Controls->segmentationTaskListWidget != nullptr
    ? m_Controls->segmentationTaskListWidget->OnPreShutdown()
    : true;
}

void QmitkSegmentationFlowControlView::SetFocus()
{
    m_Controls->btnStoreAndAccept->setFocus();
}

void QmitkSegmentationFlowControlView::CreateQtPartControl(QWidget* parent)
{
  m_Controls->setupUi(parent);
  m_Parent = parent;

  using Self = QmitkSegmentationFlowControlView;

  connect(m_Controls->btnStore, &QPushButton::clicked, this, &Self::OnStoreButtonClicked);
  connect(m_Controls->btnStoreAndAccept, &QPushButton::clicked, this, &Self::OnAcceptButtonClicked);
  connect(m_Controls->segmentationTaskListWidget, &QmitkSegmentationTaskListWidget::ActiveTaskChanged, this, &Self::OnActiveTaskChanged);
  connect(m_Controls->segmentationTaskListWidget, &QmitkSegmentationTaskListWidget::CurrentTaskChanged, this, &Self::OnCurrentTaskChanged);

  auto* prevShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_S), parent);
  connect(prevShortcut, &QShortcut::activated, this, &Self::OnStoreInterimResultShortcutActivated);

  auto* nextShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_A), parent);
  connect(nextShortcut, &QShortcut::activated, this, &Self::OnAcceptSegmentationShortcutActivated);

  m_Controls->btnStore->setIcon(berry::QtStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg")));
  m_Controls->btnStore->setVisible(false);

  m_Controls->segmentationTaskListWidget->setVisible(false);

  m_Controls->labelStored->setVisible(false);

  this->UpdateControls();

  m_OutputDir = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputdir", itksys::SystemTools::GetCurrentWorkingDirectory()));
  m_OutputDir = QDir::fromNativeSeparators(m_OutputDir);

  m_FileExtension = QString::fromStdString(mitk::BaseApplication::instance().config().getString("flow.outputextension", "nrrd"));
}

void QmitkSegmentationFlowControlView::OnStoreButtonClicked()
{
  m_Controls->segmentationTaskListWidget->SaveActiveTask(true);
}

void QmitkSegmentationFlowControlView::OnAcceptButtonClicked()
{
  if (m_Controls->segmentationTaskListWidget->isVisible())
  {
    auto* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    int activeToolId = -1;

    if (toolManager != nullptr)
      activeToolId = toolManager->GetActiveToolID();

    m_Controls->segmentationTaskListWidget->SaveActiveTask();
    m_Controls->segmentationTaskListWidget->LoadNextUnfinishedTask();

    if (toolManager != nullptr)
      toolManager->ActivateTool(activeToolId);
  }
  else
  {
    auto nodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);

    for (auto node : *nodes)
    {
      QString outputpath = m_OutputDir + "/" + QString::fromStdString(node->GetName()) + "." + m_FileExtension;
      outputpath = QDir::toNativeSeparators(QDir::cleanPath(outputpath));
      mitk::IOUtil::Save(node->GetData(), outputpath.toStdString());
    }

    m_Controls->labelStored->setVisible(true);
  }
}

void QmitkSegmentationFlowControlView::OnActiveTaskChanged(const std::optional<size_t>&)
{
  this->UpdateControls();
}

void QmitkSegmentationFlowControlView::OnCurrentTaskChanged(const std::optional<size_t>&)
{
  this->UpdateControls();
}

void QmitkSegmentationFlowControlView::UpdateControls()
{
  auto dataStorage = this->GetDataStorage();
  auto hasTaskList = !dataStorage->GetSubset(m_SegmentationTaskListPredicate)->empty();

  m_Controls->segmentationTaskListWidget->setVisible(hasTaskList);

  if (hasTaskList)
  {
    auto activeTaskIsShown = m_Controls->segmentationTaskListWidget->ActiveTaskIsShown();

    m_Controls->btnStore->setVisible(activeTaskIsShown);
    m_Controls->btnStoreAndAccept->setEnabled(activeTaskIsShown);
  }
  else
  {
    auto hasSegmentation = !dataStorage->GetSubset(m_SegmentationPredicate)->empty();

    m_Controls->btnStore->setVisible(false);
    m_Controls->btnStoreAndAccept->setEnabled(hasSegmentation);
  }
}

void QmitkSegmentationFlowControlView::NodeAdded(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}

void QmitkSegmentationFlowControlView::NodeChanged(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}

void QmitkSegmentationFlowControlView::NodeRemoved(const mitk::DataNode* node)
{
  if (dynamic_cast<const mitk::LabelSetImage*>(node->GetData()) != nullptr)
    this->UpdateControls();
}

void QmitkSegmentationFlowControlView::OnStoreInterimResultShortcutActivated()
{
  m_Controls->btnStore->click();
}

void QmitkSegmentationFlowControlView::OnAcceptSegmentationShortcutActivated()
{
  m_Controls->btnStoreAndAccept->click();
}
