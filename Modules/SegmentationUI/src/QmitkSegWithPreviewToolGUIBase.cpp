/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkSegWithPreviewToolGUIBase.h>

#include <QmitkRun.h>

#include <QApplication>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include <deque>

namespace
{
  // Functions posted while the GUI thread waited for a blocking task, in the
  // order they were posted. Used on the GUI thread only. Never destroyed, as
  // what is left at exit may hold objects of libraries unloaded before.
  std::deque<std::function<void()>>& DeferredFunctions()
  {
    static auto* functions = new std::deque<std::function<void()>>;
    return *functions;
  }

  void RunDeferredFunctions()
  {
    if (QmitkIsWaitingForBlockingTask())
    {
      QTimer::singleShot(100, QCoreApplication::instance(), &RunDeferredFunctions);
      return;
    }

    auto& deferred = DeferredFunctions();

    while (!deferred.empty())
    {
      auto function = std::move(deferred.front());
      deferred.pop_front();
      function();
    }
  }

  // Not while the GUI thread waits for a blocking task like a save, whose
  // event loop keeps turning: the end of the work of a tool can change the
  // segmentation that the task reads. In order, so that the end does not
  // overtake a progress report of the same work.
  void RunOrDefer(std::function<void()> function)
  {
    auto& deferred = DeferredFunctions();

    if (!QmitkIsWaitingForBlockingTask() && deferred.empty())
    {
      function();
      return;
    }

    deferred.push_back(std::move(function));

    if (deferred.size() == 1)
      QTimer::singleShot(100, QCoreApplication::instance(), &RunDeferredFunctions);
  }

  // Queues a function in the event loop of the application, which runs on the
  // GUI thread. Callable from any thread.
  void PostToGUIThread(std::function<void()> function)
  {
    if (auto* application = QCoreApplication::instance(); application != nullptr)
    {
      QMetaObject::invokeMethod(application, [function = std::move(function)]() mutable {
        RunOrDefer(std::move(function));
      }, Qt::QueuedConnection);
    }
  }
}

bool DefaultEnableConfirmSegBtnFunction(bool enabled)
{
  return enabled;
}

QmitkSegWithPreviewToolGUIBase::QmitkSegWithPreviewToolGUIBase(bool mode2D, bool enableTimeSteps)
  : QmitkToolGUI(),
    m_EnableConfirmSegBtnFnc(DefaultEnableConfirmSegBtnFunction),
    m_Mode2D(mode2D),
    m_EnableProcessingOfAllTimeSteps(enableTimeSteps)
{
  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkSegWithPreviewToolGUIBase::~QmitkSegWithPreviewToolGUIBase()
{
  if (m_Tool.IsNotNull())
  {
    m_Tool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSegWithPreviewToolGUIBase, bool>(this, &QmitkSegWithPreviewToolGUIBase::BusyStateChanged);
  }

  this->RestoreBusyCursor();
}

void QmitkSegWithPreviewToolGUIBase::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_Tool.IsNotNull())
  {
    this->DisconnectOldTool(m_Tool);
  }

  m_Tool = dynamic_cast<mitk::SegWithPreviewTool*>(tool);

  if (nullptr == m_MainLayout)
  {
    // create the visible widgets
    m_MainLayout = new QVBoxLayout(this);
    m_MainLayout->setContentsMargins(0, 0, 0, 0);
    m_ConfirmSegBtn = new QPushButton("Confirm Segmentation", this);
    connect(m_ConfirmSegBtn, SIGNAL(clicked()), this, SLOT(OnAcceptPreview()));

    m_CheckIgnoreLocks = new QCheckBox("Ignore label locks", this);
    m_CheckIgnoreLocks->setChecked(m_Tool->GetOverwriteStyle() == mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    m_CheckIgnoreLocks->setToolTip("If checked, the lock state of labels will be ignored when the preview segmentation is confermed. Thus also locked label pixels can be changed by the operation.");

    m_CheckMerge = new QCheckBox("Merge with existing content", this);
    m_CheckMerge->setChecked(m_Tool->GetMergeStyle()==mitk::MultiLabelSegmentation::MergeStyle::Merge);
    m_CheckMerge->setToolTip("If checked, the preview segmentation will be merged with the existing segmentation into a union. If unchecked, the preview content will replace the old segmentation");

    m_CheckProcessAll = new QCheckBox("Process all time steps", this);
    m_CheckProcessAll->setChecked(false);
    m_CheckProcessAll->setToolTip("Process all time steps of the dynamic segmentation and not just the currently visible time step.");
    m_CheckProcessAll->setVisible(!m_Mode2D && m_EnableProcessingOfAllTimeSteps);
    //remark: keep m_CheckProcessAll deactivated in 2D because in this refactoring
    //it should be kept to the status quo and it was not clear how interpolation
    //would behave. As soon as it is sorted out we can remove that "feature switch"
    //or the comment.

    // Create the preview visibility checkbox, its heading, and the opacity
    // slider before InitializeUI() so derived tool GUIs can attach shortcuts to
    // them, just as they can to m_ConfirmSegBtn (created above). The layout is
    // assembled further below.
    m_PreviewLabel = new QLabel("Preview visibility", this);

    m_PreviewVisibleCheckBox = new QCheckBox(this);
    m_PreviewVisibleCheckBox->setChecked(m_Tool->GetPreviewVisibility());
    m_PreviewVisibleCheckBox->setToolTip("Toggle visibility of the preview segmentation.");
    connect(m_PreviewVisibleCheckBox, &QCheckBox::toggled,
            this, &QmitkSegWithPreviewToolGUIBase::OnPreviewVisibilityToggled);

    m_PreviewOpacitySlider = new QSlider(Qt::Horizontal, this);
    m_PreviewOpacitySlider->setRange(0, 100);
    m_PreviewOpacitySlider->setValue(static_cast<int>(m_Tool->GetPreviewOpacity() * 100));
    m_PreviewOpacitySlider->setToolTip("Adjust the opacity of the preview segmentation.");
    m_PreviewOpacitySlider->setEnabled(m_Tool->GetPreviewVisibility());
    connect(m_PreviewOpacitySlider, &QSlider::valueChanged,
            this, &QmitkSegWithPreviewToolGUIBase::OnPreviewOpacityChanged);

    this->InitializeUI(m_MainLayout);

    m_MainLayout->addWidget(m_ConfirmSegBtn);

    auto* optionsLayout = new QHBoxLayout();
    optionsLayout->setContentsMargins(0, 0, 0, 0);

    auto* checkBoxLayout = new QVBoxLayout();
    checkBoxLayout->setContentsMargins(0, 0, 0, 0);
    checkBoxLayout->addWidget(m_CheckIgnoreLocks);
    checkBoxLayout->addWidget(m_CheckMerge);
    checkBoxLayout->addWidget(m_CheckProcessAll);
    optionsLayout->addLayout(checkBoxLayout);

    optionsLayout->addSpacing(32);

    auto* opacityLayout = new QVBoxLayout();
    opacityLayout->setContentsMargins(0, 0, 0, 0);
    opacityLayout->addWidget(m_PreviewLabel);

    auto* sliderRow = new QHBoxLayout();
    sliderRow->setContentsMargins(0, 0, 0, 0);
    sliderRow->addWidget(m_PreviewVisibleCheckBox);
    sliderRow->addWidget(m_PreviewOpacitySlider);
    opacityLayout->addLayout(sliderRow);

    optionsLayout->addLayout(opacityLayout);

    m_MainLayout->addLayout(optionsLayout);
  }

  if (m_Tool.IsNotNull())
  {
    this->ConnectNewTool(m_Tool);
  }
}

void QmitkSegWithPreviewToolGUIBase::OnAcceptPreview()
{
  if (m_Tool.IsNotNull())
  {
    if (m_CheckIgnoreLocks->isChecked())
    {
      m_Tool->SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    }
    else
    {
      m_Tool->SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    }

    if (m_CheckMerge->isChecked())
    {
      m_Tool->SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle::Merge);
    }
    else
    {
      m_Tool->SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle::Replace);
    }

    m_Tool->SetCreateAllTimeSteps(m_CheckProcessAll->isChecked());

    m_ConfirmSegBtn->setEnabled(false);
    m_Tool->ConfirmSegmentation();
  }
}

void QmitkSegWithPreviewToolGUIBase::OnPreviewOpacityChanged(int value)
{
  if (m_Tool.IsNotNull())
  {
    m_Tool->SetPreviewOpacity(value / 100.0f);
  }
}

void QmitkSegWithPreviewToolGUIBase::OnPreviewVisibilityToggled(bool checked)
{
  if (m_Tool.IsNotNull())
  {
    m_Tool->SetPreviewVisibility(checked);
  }

  if (nullptr != m_PreviewOpacitySlider)
  {
    m_PreviewOpacitySlider->setEnabled(checked);
  }
}

void QmitkSegWithPreviewToolGUIBase::DisconnectOldTool(mitk::SegWithPreviewTool* oldTool)
{
  oldTool->CurrentlyBusy -= mitk::MessageDelegate1<QmitkSegWithPreviewToolGUIBase, bool>(this, &QmitkSegWithPreviewToolGUIBase::BusyStateChanged);
  this->RestoreBusyCursor();
}

void QmitkSegWithPreviewToolGUIBase::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  newTool->SetGUIThreadPoster(PostToGUIThread);

  newTool->CurrentlyBusy +=
    mitk::MessageDelegate1<QmitkSegWithPreviewToolGUIBase, bool>(this, &QmitkSegWithPreviewToolGUIBase::BusyStateChanged);

  m_CheckProcessAll->setVisible(
    !m_Mode2D &&
    m_EnableProcessingOfAllTimeSteps &&
    newTool->GetTargetSegmentationNode()->GetData()->GetTimeSteps() > 1);

  // An update of an earlier activation can still be ending. Its end enables
  // the widgets through BusyStateChanged().
  this->EnableWidgets(!newTool->IsUpdating());
}

void QmitkSegWithPreviewToolGUIBase::InitializeUI(QBoxLayout* /*mainLayout*/)
{
  //default implementation does nothing
}

void QmitkSegWithPreviewToolGUIBase::BusyStateChanged(bool isBusy)
{
  if (!isBusy)
  {
    this->RestoreBusyCursor();
  }
  else if (!m_BusyCursorSet)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    m_BusyCursorSet = true;
  }
  this->EnableWidgets(!isBusy);
}

void QmitkSegWithPreviewToolGUIBase::RestoreBusyCursor()
{
  if (!m_BusyCursorSet)
    return;

  QApplication::restoreOverrideCursor();
  m_BusyCursorSet = false;
}

void QmitkSegWithPreviewToolGUIBase::EnableWidgets(bool enabled)
{
  if (nullptr != m_MainLayout)
  {
    if (nullptr != m_ConfirmSegBtn)
    {
      m_ConfirmSegBtn->setEnabled(m_EnableConfirmSegBtnFnc(enabled));
    }
    if (nullptr != m_CheckIgnoreLocks)
    {
      m_CheckIgnoreLocks->setEnabled(enabled);
    }
    if (nullptr != m_CheckMerge)
    {
      m_CheckMerge->setEnabled(enabled);
    }
    if (nullptr != m_CheckProcessAll)
    {
      m_CheckProcessAll->setEnabled(enabled);
    }
    if (nullptr != m_PreviewVisibleCheckBox)
    {
      m_PreviewVisibleCheckBox->setEnabled(enabled);
    }
    if (nullptr != m_PreviewOpacitySlider)
    {
      m_PreviewOpacitySlider->setEnabled(
        enabled && m_PreviewVisibleCheckBox != nullptr && m_PreviewVisibleCheckBox->isChecked());
    }
  }
}

void QmitkSegWithPreviewToolGUIBase::SetMergeStyle(mitk::MultiLabelSegmentation::MergeStyle mergeStyle)
{
  if (nullptr != m_CheckMerge)
  {
    m_CheckMerge->setChecked(mergeStyle == mitk::MultiLabelSegmentation::MergeStyle::Merge);
  }
}

void QmitkSegWithPreviewToolGUIBase::SetOverwriteStyle(mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle)
{
  if (nullptr != m_CheckIgnoreLocks)
  {
    m_CheckIgnoreLocks->setChecked(overwriteStyle == mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
  }
}

QPushButton* QmitkSegWithPreviewToolGUIBase::GetConfirmSegmentationButton() const
{
  return m_ConfirmSegBtn;
}

QCheckBox* QmitkSegWithPreviewToolGUIBase::GetPreviewVisibilityCheckBox() const
{
  return m_PreviewVisibleCheckBox;
}

QLabel* QmitkSegWithPreviewToolGUIBase::GetPreviewLabel() const
{
  return m_PreviewLabel;
}
