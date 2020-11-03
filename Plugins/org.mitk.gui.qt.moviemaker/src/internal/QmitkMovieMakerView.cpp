/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMovieMakerView.h"
#include <ui_QmitkMovieMakerView.h>

#include "QmitkAnimationItemDelegate.h"
#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"
#include "QmitkSliceAnimationItem.h"
#include "QmitkSliceAnimationWidget.h"
#include "QmitkTimeSliceAnimationItem.h"
#include "QmitkTimeSliceAnimationWidget.h"

#include <berryPlatform.h>

#include <mitkGL.h>

#include <QmitkFFmpegWriter.h>

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>

#include <array>

namespace
{
  QmitkAnimationItem* CreateDefaultAnimation(const QString& widgetKey)
  {
    if (widgetKey == "Orbit")
      return new QmitkOrbitAnimationItem;

    if (widgetKey == "Slice")
      return new QmitkSliceAnimationItem;

    if (widgetKey == "Time")
      return new QmitkTimeSliceAnimationItem;

    return nullptr;
  }

  QString GetFFmpegPath()
  {
    auto preferences = berry::Platform::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.gui.qt.ext.externalprograms");

    return preferences.IsNotNull()
      ? preferences->Get("ffmpeg", "")
      : "";
  }

  void ReadPixels(std::unique_ptr<unsigned char[]>& frame, vtkRenderWindow* renderWindow, int x, int y, int width, int height)
  {
    if (nullptr == renderWindow)
      return;

    renderWindow->MakeCurrent();
    glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame.get());
  }
}

const std::string QmitkMovieMakerView::VIEW_ID = "org.mitk.views.moviemaker";

QmitkMovieMakerView::QmitkMovieMakerView()
  : m_FFmpegWriter(nullptr),
    m_Ui(new Ui::QmitkMovieMakerView),
    m_AnimationModel(nullptr),
    m_AddAnimationMenu(nullptr),
    m_RecordMenu(nullptr),
    m_Timer(nullptr),
    m_TotalDuration(0.0),
    m_NumFrames(0),
    m_CurrentFrame(0)
{
}

QmitkMovieMakerView::~QmitkMovieMakerView()
{
}

void QmitkMovieMakerView::CreateQtPartControl(QWidget* parent)
{
  m_FFmpegWriter = new QmitkFFmpegWriter(parent);

  m_Ui->setupUi(parent);

  this->InitializeAnimationWidgets();
  this->InitializeAnimationTreeViewWidgets();
  this->InitializePlaybackAndRecordWidgets();
  this->InitializeTimer(parent);

  m_Ui->animationWidgetGroupBox->setVisible(false);
}

void QmitkMovieMakerView::InitializeAnimationWidgets()
{
  m_AnimationWidgets["Orbit"] = new QmitkOrbitAnimationWidget;
  m_AnimationWidgets["Slice"] = new QmitkSliceAnimationWidget;
  m_AnimationWidgets["Time"] = new QmitkTimeSliceAnimationWidget;

  for (const auto& widget : m_AnimationWidgets)
  {
    if (nullptr != widget.second)
    {
      widget.second->setVisible(false);
      m_Ui->animationWidgetGroupBoxLayout->addWidget(widget.second);
    }
  }

  this->ConnectAnimationWidgets();
}

void QmitkMovieMakerView::InitializeAnimationTreeViewWidgets()
{
  this->InitializeAnimationModel();
  this->InitializeAddAnimationMenu();
  this->ConnectAnimationTreeViewWidgets();
}

void QmitkMovieMakerView::InitializePlaybackAndRecordWidgets()
{
  this->InitializeRecordMenu();
  this->ConnectPlaybackAndRecordWidgets();
}

void QmitkMovieMakerView::InitializeAnimationModel()
{
  m_AnimationModel = new QStandardItemModel(m_Ui->animationTreeView);
  m_AnimationModel->setHorizontalHeaderLabels(QStringList() << "Animation" << "Timeline");

  m_Ui->animationTreeView->setModel(m_AnimationModel);
  m_Ui->animationTreeView->setItemDelegate(new QmitkAnimationItemDelegate(m_Ui->animationTreeView));
}

void QmitkMovieMakerView::InitializeAddAnimationMenu()
{
  m_AddAnimationMenu = new QMenu(m_Ui->addAnimationButton);

  for(const auto& widget : m_AnimationWidgets)
    m_AddAnimationMenu->addAction(widget.first);
}

void QmitkMovieMakerView::InitializeRecordMenu()
{
  std::array<std::pair<QString, QString>, 4> renderWindows = {
    std::make_pair(QStringLiteral("Axial"), QStringLiteral("stdmulti.widget0")),
    std::make_pair(QStringLiteral("Sagittal"), QStringLiteral("stdmulti.widget1")),
    std::make_pair(QStringLiteral("Coronal"), QStringLiteral("stdmulti.widget2")),
    std::make_pair(QStringLiteral("3D"), QStringLiteral("stdmulti.widget3"))
  };

  m_RecordMenu = new QMenu(m_Ui->recordButton);

  for(const auto& renderWindow : renderWindows)
  {
    auto* action = new QAction(m_RecordMenu);
    action->setText(renderWindow.first);
    action->setData(renderWindow.second);

    m_RecordMenu->addAction(action);
  }
}

void QmitkMovieMakerView::InitializeTimer(QWidget* parent)
{
  m_Timer = new QTimer(parent);

  this->OnFPSSpinBoxValueChanged(m_Ui->fpsSpinBox->value());
  this->ConnectTimer();
}

void QmitkMovieMakerView::ConnectAnimationTreeViewWidgets()
{
  connect(m_AnimationModel, &QStandardItemModel::rowsInserted, this, &QmitkMovieMakerView::OnAnimationTreeViewRowsInserted);
  connect(m_AnimationModel, &QStandardItemModel::rowsRemoved, this, &QmitkMovieMakerView::OnAnimationTreeViewRowsRemoved);
  connect(m_Ui->animationTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QmitkMovieMakerView::OnAnimationTreeViewSelectionChanged);
  connect(m_Ui->moveAnimationUpButton, &QToolButton::clicked, this, &QmitkMovieMakerView::OnMoveAnimationUpButtonClicked);
  connect(m_Ui->moveAnimationDownButton, &QToolButton::clicked, this, &QmitkMovieMakerView::OnMoveAnimationDownButtonClicked);
  connect(m_Ui->addAnimationButton, &QToolButton::clicked, this, &QmitkMovieMakerView::OnAddAnimationButtonClicked);
  connect(m_Ui->removeAnimationButton, &QToolButton::clicked, this, &QmitkMovieMakerView::OnRemoveAnimationButtonClicked);
}

void QmitkMovieMakerView::ConnectAnimationWidgets()
{
  connect(m_Ui->startComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnStartComboBoxCurrentIndexChanged(int)));
  connect(m_Ui->durationSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDurationSpinBoxValueChanged(double)));
  connect(m_Ui->delaySpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnDelaySpinBoxValueChanged(double)));
}

void QmitkMovieMakerView::ConnectPlaybackAndRecordWidgets()
{
  connect(m_Ui->playButton, &QToolButton::toggled, this, &QmitkMovieMakerView::OnPlayButtonToggled);
  connect(m_Ui->stopButton, &QToolButton::clicked, this, &QmitkMovieMakerView::OnStopButtonClicked);
  connect(m_Ui->recordButton, &QToolButton::clicked, this, &QmitkMovieMakerView::OnRecordButtonClicked);
  connect(m_Ui->fpsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnFPSSpinBoxValueChanged(int)));
}

void QmitkMovieMakerView::ConnectTimer()
{
  connect(m_Timer, &QTimer::timeout, this, &QmitkMovieMakerView::OnTimerTimeout);
}

void QmitkMovieMakerView::SetFocus()
{
  m_Ui->addAnimationButton->setFocus();
}

void QmitkMovieMakerView::OnMoveAnimationUpButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
  {
    const int selectedRow = selection[0].top();

    if (selectedRow > 0)
      m_AnimationModel->insertRow(selectedRow - 1, m_AnimationModel->takeRow(selectedRow));
  }

  this->CalculateTotalDuration();
}

void QmitkMovieMakerView::OnMoveAnimationDownButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
  {
    const int rowCount = m_AnimationModel->rowCount();
    const int selectedRow = selection[0].top();

    if (selectedRow < rowCount - 1)
      m_AnimationModel->insertRow(selectedRow + 1, m_AnimationModel->takeRow(selectedRow));
  }

  this->CalculateTotalDuration();
}

void QmitkMovieMakerView::OnAddAnimationButtonClicked()
{
  auto action = m_AddAnimationMenu->exec(QCursor::pos());

  if (nullptr != action)
  {
    const auto key = action->text();

    m_AnimationModel->appendRow(QList<QStandardItem*>()
      << new QStandardItem(key)
      << CreateDefaultAnimation(key));

    m_Ui->playbackAndRecordingGroupBox->setEnabled(true);
  }
}

void QmitkMovieMakerView::OnPlayButtonToggled(bool checked)
{
  if (checked)
  {
    m_Ui->playButton->setIcon(QIcon(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-pause.svg"));
    m_Ui->playButton->repaint();

    m_Timer->start(static_cast<int>(1000.0 / m_Ui->fpsSpinBox->value()));
  }
  else
  {
    m_Timer->stop();

    m_Ui->playButton->setIcon(QIcon(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg"));
    m_Ui->playButton->repaint();
  }
}

void QmitkMovieMakerView::OnStopButtonClicked()
{
  m_Ui->playButton->setChecked(false);
  m_Ui->stopButton->setEnabled(false);

  m_CurrentFrame = 0;
  this->RenderCurrentFrame();
}

void QmitkMovieMakerView::OnRecordButtonClicked()
{
  if (0 == m_NumFrames || 0.0 == m_TotalDuration)
    return;

  const QString ffmpegPath = GetFFmpegPath();

  if (ffmpegPath.isEmpty())
  {
    QMessageBox::information(nullptr, "Movie Maker",
      "<p>Set path to FFmpeg (<a href=\"https://ffmpeg.org\">ffmpeg.org</a>) in preferences "
      "(Window -> Preferences... (Ctrl+P) -> External Programs) "
      "to be able to record your movies to video files.</p>");
    return;
  }

  m_FFmpegWriter->SetFFmpegPath(GetFFmpegPath());

  auto action = m_RecordMenu->exec(QCursor::pos());

  if (nullptr == action)
    return;

  auto renderWindow = mitk::BaseRenderer::GetRenderWindowByName(action->data().toString().toStdString());

  if (nullptr == renderWindow)
    return;

  const int border = 3;
  const int x = border;
  const int y = border;
  int width = renderWindow->GetSize()[0] - border * 2;
  int height = renderWindow->GetSize()[1] - border * 2;

  if (width & 1)
    --width;

  if (height & 1)
    --height;

  if (width < 16 || height < 16)
    return;

  m_FFmpegWriter->SetSize(width, height);
  m_FFmpegWriter->SetFramerate(m_Ui->fpsSpinBox->value());

  QString saveFileName = QFileDialog::getSaveFileName(nullptr, "Specify a filename", "", "Movie (*.mp4)");

  if (saveFileName.isEmpty())
    return;

  if(!saveFileName.endsWith(".mp4"))
    saveFileName += ".mp4";

  m_FFmpegWriter->SetOutputPath(saveFileName);

  try
  {
    auto frame = std::make_unique<unsigned char[]>(width * height * 3);
    m_FFmpegWriter->Start();

    for (m_CurrentFrame = 0; m_CurrentFrame < m_NumFrames; ++m_CurrentFrame)
    {
      this->RenderCurrentFrame();
      ReadPixels(frame, renderWindow, x, y, width, height);
      m_FFmpegWriter->WriteFrame(frame.get());
    }

    m_FFmpegWriter->Stop();

    m_CurrentFrame = 0;
    this->RenderCurrentFrame();
  }
  catch (const mitk::Exception& exception)
  {
    m_CurrentFrame = 0;
    this->RenderCurrentFrame();

    QMessageBox::critical(nullptr, "Movie Maker", exception.GetDescription());
  }
}

void QmitkMovieMakerView::OnRemoveAnimationButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
    m_AnimationModel->removeRow(selection[0].top());
}

void QmitkMovieMakerView::OnAnimationTreeViewRowsInserted(const QModelIndex& parent, int start, int)
{
  this->CalculateTotalDuration();

  m_Ui->animationTreeView->selectionModel()->select(
    m_AnimationModel->index(start, 0, parent),
    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void QmitkMovieMakerView::OnAnimationTreeViewRowsRemoved(const QModelIndex&, int, int)
{
  this->CalculateTotalDuration();
  this->UpdateWidgets();
}

void QmitkMovieMakerView::OnAnimationTreeViewSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  this->UpdateWidgets();
}

void QmitkMovieMakerView::OnStartComboBoxCurrentIndexChanged(int index)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != nullptr)
  {
    item->SetStartWithPrevious(index);
    this->RedrawTimeline();
    this->CalculateTotalDuration();
  }
}

void QmitkMovieMakerView::OnDurationSpinBoxValueChanged(double value)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != nullptr)
  {
    item->SetDuration(value);
    this->RedrawTimeline();
    this->CalculateTotalDuration();
  }
}

void QmitkMovieMakerView::OnDelaySpinBoxValueChanged(double value)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != nullptr)
  {
    item->SetDelay(value);
    this->RedrawTimeline();
    this->CalculateTotalDuration();
  }
}

void QmitkMovieMakerView::OnFPSSpinBoxValueChanged(int value)
{
  this->CalculateTotalDuration();
  m_Timer->setInterval(static_cast<int>(1000.0 / value));
}

void QmitkMovieMakerView::OnTimerTimeout()
{
  this->RenderCurrentFrame();

  m_CurrentFrame = std::min(m_NumFrames, m_CurrentFrame + 1);

  if (m_CurrentFrame >= m_NumFrames)
  {
    m_Ui->playButton->setChecked(false);

    m_CurrentFrame = 0;
    this->RenderCurrentFrame();
  }

  m_Ui->stopButton->setEnabled(m_CurrentFrame != 0);
}

void QmitkMovieMakerView::RenderCurrentFrame()
{
  const double deltaT = m_TotalDuration / (m_NumFrames - 1);
  const auto activeAnimations = this->GetActiveAnimations(m_CurrentFrame * deltaT);

  for (const auto& animation : activeAnimations)
  {
    const auto nextActiveAnimations = this->GetActiveAnimations((m_CurrentFrame + 1) * deltaT);
    bool lastFrameForAnimation = true;

    for (const auto& nextAnimation : nextActiveAnimations)
    {
      if (nextAnimation.first == animation.first)
      {
        lastFrameForAnimation = false;
        break;
      }
    }

    animation.first->Animate(!lastFrameForAnimation
      ? animation.second
      : 1.0);
  }

  mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void QmitkMovieMakerView::UpdateWidgets()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (selection.isEmpty())
  {
    m_Ui->moveAnimationUpButton->setEnabled(false);
    m_Ui->moveAnimationDownButton->setEnabled(false);
    m_Ui->removeAnimationButton->setEnabled(false);
    m_Ui->playbackAndRecordingGroupBox->setEnabled(false);

    this->HideCurrentAnimationWidget();
  }
  else
  {
    const int rowCount = m_AnimationModel->rowCount();
    const int selectedRow = selection[0].top();

    m_Ui->moveAnimationUpButton->setEnabled(rowCount > 1 && selectedRow != 0);
    m_Ui->moveAnimationDownButton->setEnabled(rowCount > 1 && selectedRow < rowCount - 1);
    m_Ui->removeAnimationButton->setEnabled(true);
    m_Ui->playbackAndRecordingGroupBox->setEnabled(true);

    this->ShowAnimationWidget(dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(selectedRow, 1)));
  }

  this->UpdateAnimationWidgets();
}

void QmitkMovieMakerView::UpdateAnimationWidgets()
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != nullptr)
  {
    m_Ui->startComboBox->setCurrentIndex(item->GetStartWithPrevious());
    m_Ui->durationSpinBox->setValue(item->GetDuration());
    m_Ui->delaySpinBox->setValue(item->GetDelay());

    m_Ui->animationGroupBox->setEnabled(true);
  }
  else
  {
    m_Ui->animationGroupBox->setEnabled(false);
  }
}

void QmitkMovieMakerView::HideCurrentAnimationWidget()
{
  if (m_Ui->animationWidgetGroupBox->isVisible())
  {
    m_Ui->animationWidgetGroupBox->setVisible(false);

    int numWidgets = m_Ui->animationWidgetGroupBoxLayout->count();

    for (int i = 0; i < numWidgets; ++i)
      m_Ui->animationWidgetGroupBoxLayout->itemAt(i)->widget()->setVisible(false);
  }
}

void QmitkMovieMakerView::ShowAnimationWidget(QmitkAnimationItem* animationItem)
{
  this->HideCurrentAnimationWidget();

  if (animationItem == nullptr)
    return;

  const QString widgetKey = animationItem->GetWidgetKey();
  auto animationWidgetIter = m_AnimationWidgets.find(widgetKey);
  auto animationWidget = m_AnimationWidgets.end() != animationWidgetIter
    ? animationWidgetIter->second
    : nullptr;

  if (nullptr != animationWidget)
  {
    m_Ui->animationWidgetGroupBox->setTitle(widgetKey);
    animationWidget->SetAnimationItem(animationItem);
    animationWidget->setVisible(true);
  }

  m_Ui->animationWidgetGroupBox->setVisible(animationWidget != nullptr);
}

void QmitkMovieMakerView::RedrawTimeline()
{
  if (m_AnimationModel->rowCount() > 1)
  {
    m_Ui->animationTreeView->dataChanged(
      m_AnimationModel->index(0, 1),
      m_AnimationModel->index(m_AnimationModel->rowCount() - 1, 1));
  }
}

QmitkAnimationItem* QmitkMovieMakerView::GetSelectedAnimationItem() const
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  return !selection.isEmpty()
    ? dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(selection[0].top(), 1))
    : nullptr;
}

void QmitkMovieMakerView::CalculateTotalDuration()
{
  const int rowCount = m_AnimationModel->rowCount();

  double totalDuration = 0.0;
  double previousStart = 0.0;

  for (int i = 0; i < rowCount; ++i)
  {
   auto item = dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(i, 1));

    if (nullptr == item)
      continue;

    if (item->GetStartWithPrevious())
    {
      totalDuration = std::max(totalDuration, previousStart + item->GetDelay() + item->GetDuration());
    }
    else
    {
      previousStart = totalDuration;
      totalDuration += item->GetDelay() + item->GetDuration();
    }
  }

  m_TotalDuration = totalDuration;
  m_NumFrames = static_cast<int>(totalDuration * m_Ui->fpsSpinBox->value());
}

std::vector<std::pair<QmitkAnimationItem*, double>> QmitkMovieMakerView::GetActiveAnimations(double t) const
{
  const int rowCount = m_AnimationModel->rowCount();

  std::vector<std::pair<QmitkAnimationItem*, double>> activeAnimations;

  double totalDuration = 0.0;
  double previousStart = 0.0;

  for (int i = 0; i < rowCount; ++i)
  {
    QmitkAnimationItem* item = dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(i, 1));

    if (item == nullptr)
      continue;

    if (item->GetDuration() > 0.0)
    {
      double start = item->GetStartWithPrevious()
        ? previousStart + item->GetDelay()
        : totalDuration + item->GetDelay();

      if (start <= t && t <= start + item->GetDuration())
        activeAnimations.emplace_back(item, (t - start) / item->GetDuration());
    }

    if (item->GetStartWithPrevious())
    {
      totalDuration = std::max(totalDuration, previousStart + item->GetDelay() + item->GetDuration());
    }
    else
    {
      previousStart = totalDuration;
      totalDuration += item->GetDelay() + item->GetDuration();
    }
  }

  return activeAnimations;
}
