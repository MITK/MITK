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
#include "QmitkMovieMaker2View.h"
#include "QmitkOrbitAnimationItem.h"
#include "QmitkOrbitAnimationWidget.h"
#include "QmitkSliceAnimationItem.h"
#include "QmitkSliceAnimationWidget.h"
#include <ui_QmitkMovieMaker2View.h>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItemModel>
#include <mitkBaseRenderer.h>

static QmitkOrbitAnimationItem* CreateDefaultOrbitAnimation()
{
  return new QmitkOrbitAnimationItem(360.0, false);
}

static QmitkSliceAnimationItem* CreateDefaultSliceAnimation()
{
  return new QmitkSliceAnimationItem(0, 0, false);
}

static QmitkAnimationItem* CreateDefaultAnimation(const QString& widgetKey)
{
  if (widgetKey == "Orbit")
    return CreateDefaultOrbitAnimation();

  if (widgetKey == "Slice")
    return CreateDefaultSliceAnimation();

  return NULL;
}

static QString GetPrettyRenderWindowName(const QString& name)
{
  QMap<QString, QString> map;
  map["stdmulti.widget1"] = "Axial";
  map["stdmulti.widget2"] = "Sagittal";
  map["stdmulti.widget3"] = "Coronal";
  map["stdmulti.widget4"] = "3D";

  return map.contains(name)
    ? map[name]
    : name;
}

const std::string QmitkMovieMaker2View::VIEW_ID = "org.mitk.views.moviemaker2";

QmitkMovieMaker2View::QmitkMovieMaker2View()
  : m_Ui(new Ui::QmitkMovieMaker2View),
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
  this->ConnectPlaybackAndRecordWidgets();
}

void QmitkMovieMaker2View::InitializeAnimationModel()
{
  m_AnimationModel = new QStandardItemModel(m_Ui->animationTreeView);
  m_AnimationModel->setHorizontalHeaderLabels(QStringList() << "Animation" << "Timeline");
  m_Ui->animationTreeView->setModel(m_AnimationModel);

  m_Ui->animationTreeView->setItemDelegateForColumn(1, new QmitkAnimationItemDelegate(m_Ui->animationTreeView));
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
  if (m_RecordMenu == NULL)
  {
    m_RecordMenu = new QMenu(m_Ui->recordButton);
  }
  else
  {
    m_RecordMenu->clear();
  }

  const mitk::RenderingManager::RenderWindowVector& renderWindows
    = mitk::RenderingManager::GetInstance()->GetAllRegisteredRenderWindows();

  Q_FOREACH(const mitk::RenderingManager::RenderWindowVector::value_type renderWindow, renderWindows)
  {
    QString name = mitk::BaseRenderer::GetInstance(renderWindow)->GetName();
    m_RecordMenu->addAction(GetPrettyRenderWindowName(name));
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

void QmitkMovieMaker2View::OnRecordButtonClicked()
{
  this->InitializeRecordMenu();

  if (m_RecordMenu->isEmpty())
    QMessageBox::warning(NULL, QString::fromStdString(this->GetPartName()), "Could not find any render window.");

  QAction* action = m_RecordMenu->exec(QCursor::pos());

  if (action != NULL)
  {
    // TODO
  }
}

void QmitkMovieMaker2View::OnRemoveAnimationButtonClicked()
{
  const QItemSelection selection = m_Ui->animationTreeView->selectionModel()->selection();

  if (!selection.isEmpty())
  {
    m_AnimationModel->removeRow(selection[0].top());
  }
  else
  {
    m_Ui->playbackAndRecordingGroupBox->setEnabled(false);
  }
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

    this->HideCurrentAnimationWidget();
  }
  else
  {
    const int rowCount = m_AnimationModel->rowCount();
    const int selectedRow = selection[0].top();

    m_Ui->moveAnimationUpButton->setEnabled(rowCount > 1 && selectedRow != 0);
    m_Ui->moveAnimationDownButton->setEnabled(rowCount > 1 && selectedRow < rowCount - 1);
    m_Ui->removeAnimationButton->setEnabled(true);

    this->ShowAnimationWidget(m_AnimationModel->item(selectedRow, 1)->data(QmitkAnimationItem::WidgetKeyRole).toString());
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

void QmitkMovieMaker2View::ShowAnimationWidget(const QString& key)
{
  this->HideCurrentAnimationWidget();

  QWidget* widget = NULL;

  if (m_AnimationWidgets.contains(key))
  {
    widget = m_AnimationWidgets[key];

    if (widget != NULL)
    {
      m_Ui->animationWidgetGroupBox->setTitle(key);
      widget->setVisible(true);
    }
  }

  m_Ui->animationWidgetGroupBox->setVisible(widget != NULL);
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
