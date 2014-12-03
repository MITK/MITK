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

#include "QmitkAnimationItemDelegate.h"
#include "QmitkFFmpegWriter.h"
#include "QmitkMovieMakerView.h"
#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"
#include "QmitkSliceAnimationItem.h"
#include "QmitkSliceAnimationWidget.h"
#include "QmitkTimeSliceAnimationWidget.h"
#include "QmitkTimeSliceAnimationItem.h"
#include <ui_QmitkMovieMakerView.h>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTimer>
#include <mitkBaseRenderer.h>
#include <mitkGL.h>

static QmitkAnimationItem* CreateDefaultAnimation(const QString& widgetKey)
{
  if (widgetKey == "Orbit")
    return new QmitkOrbitAnimationItem;

  if (widgetKey == "Slice")
    return new QmitkSliceAnimationItem;

  if (widgetKey == "Time")
    return new QmitkTimeSliceAnimationItem;

  return NULL;
}

static QString GetFFmpegPath()
{
  berry::IPreferencesService::Pointer preferencesService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IPreferences::Pointer preferences = preferencesService->GetSystemPreferences()->Node("/org.mitk.gui.qt.ext.externalprograms");

  return QString::fromStdString(preferences->Get("ffmpeg", ""));
}

static unsigned char* ReadPixels(vtkRenderWindow* renderWindow, int x, int y, int width, int height)
{
  if (renderWindow == NULL)
    return NULL;

  unsigned char* frame = new unsigned char[width * height * 3];

  renderWindow->MakeCurrent();
  glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, frame);

  return frame;
}

const std::string QmitkMovieMakerView::VIEW_ID = "org.mitk.views.moviemaker";

QmitkMovieMakerView::QmitkMovieMakerView()
  : m_FFmpegWriter(NULL),
    m_Ui(new Ui::QmitkMovieMakerView),
    m_AnimationModel(NULL),
    m_AddAnimationMenu(NULL),
    m_RecordMenu(NULL),
    m_Timer(NULL),
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

  Q_FOREACH(QWidget* widget, m_AnimationWidgets.values())
  {
    if (widget != NULL)
    {
      widget->setVisible(false);
      m_Ui->animationWidgetGroupBoxLayout->addWidget(widget);
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

  Q_FOREACH(const QString& key, m_AnimationWidgets.keys())
  {
    m_AddAnimationMenu->addAction(key);
  }
}

void QmitkMovieMakerView::InitializeRecordMenu()
{
  typedef QPair<QString, QString> PairOfStrings;

  m_RecordMenu = new QMenu(m_Ui->recordButton);

  QVector<PairOfStrings> renderWindows;
  renderWindows.push_back(qMakePair(QString("Axial"), QString("stdmulti.widget1")));
  renderWindows.push_back(qMakePair(QString("Sagittal"), QString("stdmulti.widget2")));
  renderWindows.push_back(qMakePair(QString("Coronal"), QString("stdmulti.widget3")));
  renderWindows.push_back(qMakePair(QString("3D"), QString("stdmulti.widget4")));

  Q_FOREACH(const PairOfStrings& renderWindow, renderWindows)
  {
    QAction* action = new QAction(m_RecordMenu);
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
  this->connect(m_AnimationModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
    this, SLOT(OnAnimationTreeViewRowsInserted(const QModelIndex&, int, int)));

  this->connect(m_AnimationModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
    this, SLOT(OnAnimationTreeViewRowsRemoved(const QModelIndex&, int, int)));

  this->connect(m_Ui->animationTreeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
    this, SLOT(OnAnimationTreeViewSelectionChanged(const QItemSelection&, const QItemSelection&)));

  this->connect(m_Ui->moveAnimationUpButton, SIGNAL(clicked()),
    this, SLOT(OnMoveAnimationUpButtonClicked()));

  this->connect(m_Ui->moveAnimationDownButton, SIGNAL(clicked()),
    this, SLOT(OnMoveAnimationDownButtonClicked()));

  this->connect(m_Ui->addAnimationButton, SIGNAL(clicked()),
    this, SLOT(OnAddAnimationButtonClicked()));

  this->connect(m_Ui->removeAnimationButton, SIGNAL(clicked()),
    this, SLOT(OnRemoveAnimationButtonClicked()));
}

void QmitkMovieMakerView::ConnectAnimationWidgets()
{
  this->connect(m_Ui->startComboBox, SIGNAL(currentIndexChanged(int)),
    this, SLOT(OnStartComboBoxCurrentIndexChanged(int)));

  this->connect(m_Ui->durationSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(OnDurationSpinBoxValueChanged(double)));

  this->connect(m_Ui->delaySpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(OnDelaySpinBoxValueChanged(double)));
}

void QmitkMovieMakerView::ConnectPlaybackAndRecordWidgets()
{
  this->connect(m_Ui->playButton, SIGNAL(toggled(bool)),
    this, SLOT(OnPlayButtonToggled(bool)));

  this->connect(m_Ui->stopButton, SIGNAL(clicked()),
    this, SLOT(OnStopButtonClicked()));

  this->connect(m_Ui->recordButton, SIGNAL(clicked()),
    this, SLOT(OnRecordButtonClicked()));

  this->connect(m_Ui->fpsSpinBox, SIGNAL(valueChanged(int)),
    this, SLOT(OnFPSSpinBoxValueChanged(int)));
}

void QmitkMovieMakerView::ConnectTimer()
{
  this->connect(m_Timer, SIGNAL(timeout()),
    this, SLOT(OnTimerTimeout()));
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
  QAction* action = m_AddAnimationMenu->exec(QCursor::pos());

  if (action != NULL)
  {
    const QString widgetKey = action->text();

    m_AnimationModel->appendRow(QList<QStandardItem*>()
      << new QStandardItem(widgetKey)
      << CreateDefaultAnimation(widgetKey));

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

void QmitkMovieMakerView::OnRecordButtonClicked() // TODO: Refactor
{
  const QString ffmpegPath = GetFFmpegPath();

  if (ffmpegPath.isEmpty())
  {
    QMessageBox::information(NULL, "Movie Maker",
      "<p>Set path to FFmpeg<sup>1</sup> or Libav<sup>2</sup> (avconv) in preferences (Window -> Preferences... (Ctrl+P) -> External Programs) to be able to record your movies to video files.</p>"
      "<p>If you are using Linux, chances are good that either FFmpeg or Libav is included in the official package repositories.</p>"
      "<p>[1] <a href=\"https://www.ffmpeg.org/download.html\">Download FFmpeg from ffmpeg.org</a><br/>"
      "[2] <a href=\"https://libav.org/download.html\">Download Libav from libav.org</a></p>");
    return;
  }

  m_FFmpegWriter->SetFFmpegPath(GetFFmpegPath());

  QAction* action = m_RecordMenu->exec(QCursor::pos());

  if (action == NULL)
    return;

  vtkRenderWindow* renderWindow = mitk::BaseRenderer::GetRenderWindowByName(action->data().toString().toStdString());

  if (renderWindow == NULL)
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

  QString saveFileName = QFileDialog::getSaveFileName(NULL, "Specify a filename", "", "Movie (*.mp4)");

  if (saveFileName.isEmpty())
    return;

  if(!saveFileName.endsWith(".mp4"))
    saveFileName += ".mp4";

  m_FFmpegWriter->SetOutputPath(saveFileName);

  try
  {
    m_FFmpegWriter->Start();

    for (m_CurrentFrame = 0; m_CurrentFrame < m_NumFrames; ++m_CurrentFrame)
    {
      this->RenderCurrentFrame();

      renderWindow->MakeCurrent();
      unsigned char* frame = ReadPixels(renderWindow, x, y, width, height);
      m_FFmpegWriter->WriteFrame(frame);
      delete[] frame;
    }

    m_FFmpegWriter->Stop();

    m_CurrentFrame = 0;
    this->RenderCurrentFrame();
  }
  catch (const mitk::Exception& exception)
  {
    m_CurrentFrame = 0;
    this->RenderCurrentFrame();

    QMessageBox::critical(NULL, "Movie Maker", exception.GetDescription());
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

  if (item != NULL)
  {
    item->SetStartWithPrevious(index);
    this->RedrawTimeline();
    this->CalculateTotalDuration();
  }
}

void QmitkMovieMakerView::OnDurationSpinBoxValueChanged(double value)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != NULL)
  {
    item->SetDuration(value);
    this->RedrawTimeline();
    this->CalculateTotalDuration();
  }
}

void QmitkMovieMakerView::OnDelaySpinBoxValueChanged(double value)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != NULL)
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
  typedef QPair<QmitkAnimationItem*, double> AnimationInterpolationFactorPair;

  const double deltaT = m_TotalDuration / (m_NumFrames - 1);
  const QVector<AnimationInterpolationFactorPair> activeAnimations = this->GetActiveAnimations(m_CurrentFrame * deltaT);

  Q_FOREACH(const AnimationInterpolationFactorPair& animation, activeAnimations)
  {
    const QVector<AnimationInterpolationFactorPair> nextActiveAnimations = this->GetActiveAnimations((m_CurrentFrame + 1) * deltaT);
    bool lastFrameForAnimation = true;

    Q_FOREACH(const AnimationInterpolationFactorPair& nextAnimation, nextActiveAnimations)
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

  if (item != NULL)
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

  if (animationItem == NULL)
    return;

  const QString widgetKey = animationItem->GetWidgetKey();
  QmitkAnimationWidget* animationWidget = NULL;

  if (m_AnimationWidgets.contains(widgetKey))
  {
    animationWidget = m_AnimationWidgets[widgetKey];

    if (animationWidget != NULL)
    {
      m_Ui->animationWidgetGroupBox->setTitle(widgetKey);
      animationWidget->SetAnimationItem(animationItem);
      animationWidget->setVisible(true);
    }
  }

  m_Ui->animationWidgetGroupBox->setVisible(animationWidget != NULL);
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
    : NULL;
}

void QmitkMovieMakerView::CalculateTotalDuration()
{
  const int rowCount = m_AnimationModel->rowCount();

  double totalDuration = 0.0;
  double previousStart = 0.0;

  for (int i = 0; i < rowCount; ++i)
  {
    QmitkAnimationItem* item = dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(i, 1));

    if (item == NULL)
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

  m_TotalDuration = totalDuration; // TODO totalDuration == 0
  m_NumFrames = static_cast<int>(totalDuration * m_Ui->fpsSpinBox->value()); // TODO numFrames < 2
}

QVector<QPair<QmitkAnimationItem*, double> > QmitkMovieMakerView::GetActiveAnimations(double t) const
{
  const int rowCount = m_AnimationModel->rowCount();

  QVector<QPair<QmitkAnimationItem*, double> > activeAnimations;

  double totalDuration = 0.0;
  double previousStart = 0.0;

  for (int i = 0; i < rowCount; ++i)
  {
    QmitkAnimationItem* item = dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(i, 1));

    if (item == NULL)
      continue;

    if (item->GetDuration() > 0.0)
    {
      double start = item->GetStartWithPrevious()
        ? previousStart + item->GetDelay()
        : totalDuration + item->GetDelay();

      if (start <= t && t <= start + item->GetDuration())
        activeAnimations.push_back(qMakePair(item, (t - start) / item->GetDuration()));
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
