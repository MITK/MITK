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
#include "QmitkMovieMaker2View.h"
#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"
#include "QmitkSliceAnimationItem.h"
#include "QmitkSliceAnimationWidget.h"
#include <ui_QmitkMovieMaker2View.h>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <mitkBaseRenderer.h>
#include <mitkGL.h>

static QmitkAnimationItem* CreateDefaultAnimation(const QString& widgetKey)
{
  if (widgetKey == "Orbit")
    return new QmitkOrbitAnimationItem(360.0, false);

  if (widgetKey == "Slice")
    return new QmitkSliceAnimationItem(0, 0, 999, false);

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

const std::string QmitkMovieMaker2View::VIEW_ID = "org.mitk.views.moviemaker2";

QmitkMovieMaker2View::QmitkMovieMaker2View()
  : m_FFmpegWriter(NULL),
    m_Ui(new Ui::QmitkMovieMaker2View),
    m_AnimationModel(NULL),
    m_AddAnimationMenu(NULL),
    m_RecordMenu(NULL)
{
}

QmitkMovieMaker2View::~QmitkMovieMaker2View()
{
}

void QmitkMovieMaker2View::CreateQtPartControl(QWidget* parent)
{
  m_FFmpegWriter = new QmitkFFmpegWriter(parent);

  m_Ui->setupUi(parent);

  this->InitializeAnimationWidgets();
  this->InitializeAnimationTreeViewWidgets();
  this->InitializePlaybackAndRecordWidgets();

  m_Ui->animationWidgetGroupBox->setVisible(false);
}

void QmitkMovieMaker2View::InitializeAnimationWidgets()
{
  m_AnimationWidgets["Orbit"] = new QmitkOrbitAnimationWidget;
  m_AnimationWidgets["Slice"] = new QmitkSliceAnimationWidget;

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

void QmitkMovieMaker2View::InitializeAnimationTreeViewWidgets()
{
  this->InitializeAnimationModel();
  this->InitializeAddAnimationMenu();
  this->ConnectAnimationTreeViewWidgets();
}

void QmitkMovieMaker2View::InitializePlaybackAndRecordWidgets()
{
  this->InitializeRecordMenu();
  this->ConnectPlaybackAndRecordWidgets();
}

void QmitkMovieMaker2View::InitializeAnimationModel()
{
  m_AnimationModel = new QStandardItemModel(m_Ui->animationTreeView);
  m_AnimationModel->setHorizontalHeaderLabels(QStringList() << "Animation" << "Timeline");
  m_Ui->animationTreeView->setModel(m_AnimationModel);

  m_Ui->animationTreeView->setItemDelegate(new QmitkAnimationItemDelegate(m_Ui->animationTreeView));
}

void QmitkMovieMaker2View::InitializeAddAnimationMenu()
{
  m_AddAnimationMenu = new QMenu(m_Ui->addAnimationButton);

  Q_FOREACH(const QString& key, m_AnimationWidgets.keys())
  {
    m_AddAnimationMenu->addAction(key);
  }
}

void QmitkMovieMaker2View::InitializeRecordMenu()
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

void QmitkMovieMaker2View::ConnectAnimationTreeViewWidgets()
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

void QmitkMovieMaker2View::ConnectAnimationWidgets()
{
  this->connect(m_Ui->startComboBox, SIGNAL(currentIndexChanged(int)),
    this, SLOT(OnStartComboBoxCurrentIndexChanged(int)));

  this->connect(m_Ui->durationSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(OnDurationSpinBoxValueChanged(double)));

  this->connect(m_Ui->delaySpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(OnDelaySpinBoxValueChanged(double)));
}

void QmitkMovieMaker2View::ConnectPlaybackAndRecordWidgets()
{
  this->connect(m_Ui->playButton, SIGNAL(toggled(bool)),
    this, SLOT(OnPlayButtonToggled(bool)));

  this->connect(m_Ui->recordButton, SIGNAL(clicked()),
    this, SLOT(OnRecordButtonClicked()));
}

void QmitkMovieMaker2View::SetFocus()
{
  m_Ui->addAnimationButton->setFocus();
}

void QmitkMovieMaker2View::OnMoveAnimationUpButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
  {
    const int selectedRow = selection[0].top();

    if (selectedRow > 0)
      m_AnimationModel->insertRow(selectedRow - 1, m_AnimationModel->takeRow(selectedRow));
  }
}

void QmitkMovieMaker2View::OnMoveAnimationDownButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
  {
    const int rowCount = m_AnimationModel->rowCount();
    const int selectedRow = selection[0].top();

    if (selectedRow < rowCount - 1)
      m_AnimationModel->insertRow(selectedRow + 1, m_AnimationModel->takeRow(selectedRow));
  }
}

void QmitkMovieMaker2View::OnAddAnimationButtonClicked()
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

void QmitkMovieMaker2View::OnPlayButtonToggled(bool /*checked*/)
{
  vtkRenderWindow* renderWindow = mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget4");
  mitk::Stepper* stepper = mitk::BaseRenderer::GetInstance(renderWindow)->GetCameraRotationController()->GetSlice();

  unsigned int startPos = stepper->GetPos();

  for (unsigned int i = 1; i < 30; ++i)
  {
    unsigned int newPos = startPos + 360.0 / 29.0 * i;
    if (newPos > 360)
      newPos -= 360;
    stepper->SetPos(newPos);

    mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(renderWindow);
  }
}

void QmitkMovieMaker2View::OnRecordButtonClicked() // TODO: Refactor
{
  m_FFmpegWriter->SetFFmpegPath(GetFFmpegPath());

  if (m_FFmpegWriter->GetFFmpegPath().isEmpty())
  {
    QMessageBox::critical(NULL, "Movie Maker 2", "Path to FFmpeg executable is not set in preferences!");
    return;
  }

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

  m_FFmpegWriter->Start();

  // TODO: Play animation

  for (int i = 0; i < 30; ++i)
  {
    renderWindow->MakeCurrent();
    unsigned char* frame = ReadPixels(renderWindow, x, y, width, height);
    m_FFmpegWriter->WriteFrame(frame);
    delete[] frame;
    mitk::BaseRenderer::GetInstance(renderWindow)->GetCameraRotationController()->GetSlice()->Next();
    mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(renderWindow);
  }

  m_FFmpegWriter->Stop();
}

void QmitkMovieMaker2View::OnRemoveAnimationButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
    m_AnimationModel->removeRow(selection[0].top());
}

void QmitkMovieMaker2View::OnAnimationTreeViewRowsInserted(const QModelIndex& parent, int start, int)
{
  m_Ui->animationTreeView->selectionModel()->select(
    m_AnimationModel->index(start, 0, parent),
    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void QmitkMovieMaker2View::OnAnimationTreeViewRowsRemoved(const QModelIndex&, int, int)
{
  this->UpdateWidgets();
}

void QmitkMovieMaker2View::OnAnimationTreeViewSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  this->UpdateWidgets();
}

void QmitkMovieMaker2View::OnStartComboBoxCurrentIndexChanged(int index)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != NULL)
  {
    item->SetStartWithPrevious(index);
    this->RedrawTimeline();
  }
}

void QmitkMovieMaker2View::OnDurationSpinBoxValueChanged(double value)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != NULL)
  {
    item->SetDuration(value);
    this->RedrawTimeline();
  }
}

void QmitkMovieMaker2View::OnDelaySpinBoxValueChanged(double value)
{
  QmitkAnimationItem* item = this->GetSelectedAnimationItem();

  if (item != NULL)
  {
    item->SetDelay(value);
    this->RedrawTimeline();
  }
}

void QmitkMovieMaker2View::UpdateWidgets()
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

void QmitkMovieMaker2View::UpdateAnimationWidgets()
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

void QmitkMovieMaker2View::HideCurrentAnimationWidget()
{
  if (m_Ui->animationWidgetGroupBox->isVisible())
  {
    m_Ui->animationWidgetGroupBox->setVisible(false);

    int numWidgets = m_Ui->animationWidgetGroupBoxLayout->count();

    for (int i = 0; i < numWidgets; ++i)
      m_Ui->animationWidgetGroupBoxLayout->itemAt(i)->widget()->setVisible(false);
  }
}

void QmitkMovieMaker2View::ShowAnimationWidget(QmitkAnimationItem* animationItem)
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

void QmitkMovieMaker2View::RedrawTimeline()
{
  if (m_AnimationModel->rowCount() > 1)
  {
    m_Ui->animationTreeView->dataChanged(
      m_AnimationModel->index(0, 1),
      m_AnimationModel->index(m_AnimationModel->rowCount() - 1, 1));
  }
}

QmitkAnimationItem* QmitkMovieMaker2View::GetSelectedAnimationItem() const
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  return !selection.isEmpty()
    ? dynamic_cast<QmitkAnimationItem*>(m_AnimationModel->item(selection[0].top(), 1))
    : NULL;
}
