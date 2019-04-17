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

#include "QmitkLabelSetWidget.h"

// mitk
#include <mitkAutoCropImageFilter.h>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageToSurfaceThreadedFilter.h>
#include <mitkRenderingManager.h>
#include <mitkShowSegmentationAsSurface.h>
#include <mitkSliceBasedInterpolationController.h>
#include <mitkStatusBar.h>
#include <mitkSurfaceBasedInterpolationController.h>
#include <mitkToolManagerProvider.h>

// Qmitk
#include <QmitkDataStorageComboBox.h>
#include <QmitkNewSegmentationDialog.h>
#include <QmitkStyleManager.h>
#include <QmitkSearchLabelDialog.h>

// Qt
#include <QColorDialog>
#include <QCompleter>
#include <QDateTime>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStringListModel>
#include <QWidgetAction>

// itk
#include <itksys/SystemTools.hxx>

// todo:
// berry
//#include <berryIPreferencesService.h>

QmitkLabelSetWidget::QmitkLabelSetWidget(QWidget *parent)
  : QWidget(parent), m_DataStorage(nullptr), m_Completer(nullptr), m_ToolManager(nullptr)
{
  m_Controls.setupUi(this);

  m_ColorSequenceRainbow.GoToBegin();

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);

  m_Controls.m_LabelSearchBox->setAlwaysShowClearIcon(true);
  m_Controls.m_LabelSearchBox->setShowSearchIcon(true);

  QStringList completionList;
  completionList << "";
  m_Completer = new QCompleter(completionList, this);
  m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.m_LabelSearchBox->setCompleter(m_Completer);

  connect(m_Controls.m_LabelSearchBox, SIGNAL(returnPressed()), this, SLOT(OnSearchLabel()));
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(labelListModified(const QStringList&)), this, SLOT(
  // OnLabelListModified(const QStringList&)) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(mergeLabel(int)), this, SLOT( OnMergeLabel(int)) );

  QStringListModel *completeModel = static_cast<QStringListModel *>(m_Completer->model());
  completeModel->setStringList(GetLabelStringList());

  m_Controls.m_LabelSearchBox->setEnabled(false);

  m_Controls.m_lblCaption->setText("");

  InitializeTableWidget();
}

QmitkLabelSetWidget::~QmitkLabelSetWidget() {}

void QmitkLabelSetWidget::OnTableViewContextMenuRequested(const QPoint & /*pos*/)
{
  int pixelValue = GetPixelValueOfSelectedItem();

  if (-1 == pixelValue)
    return;

  QMenu *menu = new QMenu(m_Controls.m_LabelSetTableWidget);

  if (m_Controls.m_LabelSetTableWidget->selectedItems().size() > 1)
  {
    QAction *mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge selection on current label", this);
    mergeAction->setEnabled(true);
    QObject::connect(mergeAction, SIGNAL(triggered(bool)), this, SLOT(OnMergeLabels(bool)));
    menu->addAction(mergeAction);

    QAction *removeLabelsAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove selected labels", this);
    removeLabelsAction->setEnabled(true);
    QObject::connect(removeLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnRemoveLabels(bool)));
    menu->addAction(removeLabelsAction);

    QAction *eraseLabelsAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase selected labels", this);
    eraseLabelsAction->setEnabled(true);
    QObject::connect(eraseLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnEraseLabels(bool)));
    menu->addAction(eraseLabelsAction);

    QAction *combineAndCreateSurfaceAction =
      new QAction(QIcon(":/Qmitk/CreateSurface.png"), "Combine and create a surface", this);
    combineAndCreateSurfaceAction->setEnabled(true);
    QObject::connect(
      combineAndCreateSurfaceAction, SIGNAL(triggered(bool)), this, SLOT(OnCombineAndCreateSurface(bool)));
    // menu->addAction(combineAndCreateSurfaceAction); Not implemented

    QAction *createMasksAction =
      new QAction(QIcon(":/Qmitk/CreateMask.png"), "Create a mask for each selected label", this);
    createMasksAction->setEnabled(true);
    QObject::connect(createMasksAction, SIGNAL(triggered(bool)), this, SLOT(OnCreateMasks(bool)));
    // menu->addAction(createMasksAction); Not implemented

    QAction *combineAndCreateMaskAction =
      new QAction(QIcon(":/Qmitk/CreateMask.png"), "Combine and create a mask", this);
    combineAndCreateMaskAction->setEnabled(true);
    QObject::connect(combineAndCreateMaskAction, SIGNAL(triggered(bool)), this, SLOT(OnCombineAndCreateMask(bool)));
    // menu->addAction(combineAndCreateMaskAction); Not implemented
  }
  else
  {
    QAction *renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Rename...", this);
    renameAction->setEnabled(true);
    QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
    menu->addAction(renameAction);

    QAction *removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove...", this);
    removeAction->setEnabled(true);
    QObject::connect(removeAction, SIGNAL(triggered(bool)), this, SLOT(OnRemoveLabel(bool)));
    menu->addAction(removeAction);

    QAction *eraseAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase...", this);
    eraseAction->setEnabled(true);
    QObject::connect(eraseAction, SIGNAL(triggered(bool)), this, SLOT(OnEraseLabel(bool)));
    menu->addAction(eraseAction);

    QAction *mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge...", this);
    mergeAction->setEnabled(true);
    QObject::connect(mergeAction, SIGNAL(triggered(bool)), this, SLOT(OnMergeLabel(bool)));
    menu->addAction(mergeAction);

    QAction *randomColorAction = new QAction(QIcon(":/Qmitk/RandomColor.png"), "Random color", this);
    randomColorAction->setEnabled(true);
    QObject::connect(randomColorAction, SIGNAL(triggered(bool)), this, SLOT(OnRandomColor(bool)));
    menu->addAction(randomColorAction);

    QAction *viewOnlyAction = new QAction(QIcon(":/Qmitk/visible.png"), "View only", this);
    viewOnlyAction->setEnabled(true);
    QObject::connect(viewOnlyAction, SIGNAL(triggered(bool)), this, SLOT(OnSetOnlyActiveLabelVisible(bool)));
    menu->addAction(viewOnlyAction);

    QAction *viewAllAction = new QAction(QIcon(":/Qmitk/visible.png"), "View all", this);
    viewAllAction->setEnabled(true);
    QObject::connect(viewAllAction, SIGNAL(triggered(bool)), this, SLOT(OnSetAllLabelsVisible(bool)));
    menu->addAction(viewAllAction);

    QAction *hideAllAction = new QAction(QIcon(":/Qmitk/invisible.png"), "Hide all", this);
    hideAllAction->setEnabled(true);
    QObject::connect(hideAllAction, SIGNAL(triggered(bool)), this, SLOT(OnSetAllLabelsInvisible(bool)));
    menu->addAction(hideAllAction);

    QAction *lockAllAction = new QAction(QIcon(":/Qmitk/lock.png"), "Lock all", this);
    lockAllAction->setEnabled(true);
    QObject::connect(lockAllAction, SIGNAL(triggered(bool)), this, SLOT(OnLockAllLabels(bool)));
    menu->addAction(lockAllAction);

    QAction *unlockAllAction = new QAction(QIcon(":/Qmitk/unlock.png"), "Unlock all", this);
    unlockAllAction->setEnabled(true);
    QObject::connect(unlockAllAction, SIGNAL(triggered(bool)), this, SLOT(OnUnlockAllLabels(bool)));
    menu->addAction(unlockAllAction);

    QAction *createSurfaceAction = new QAction(QIcon(":/Qmitk/CreateSurface.png"), "Create surface", this);
    createSurfaceAction->setEnabled(true);
    createSurfaceAction->setMenu(new QMenu());

    QAction *tmp1 = createSurfaceAction->menu()->addAction(QString("Detailed"));
    QAction *tmp2 = createSurfaceAction->menu()->addAction(QString("Smoothed"));

    QObject::connect(tmp1, SIGNAL(triggered(bool)), this, SLOT(OnCreateDetailedSurface(bool)));
    QObject::connect(tmp2, SIGNAL(triggered(bool)), this, SLOT(OnCreateSmoothedSurface(bool)));

    menu->addAction(createSurfaceAction);

    QAction *createMaskAction = new QAction(QIcon(":/Qmitk/CreateMask.png"), "Create mask", this);
    createMaskAction->setEnabled(true);
    QObject::connect(createMaskAction, SIGNAL(triggered(bool)), this, SLOT(OnCreateMask(bool)));

    menu->addAction(createMaskAction);

    QAction *createCroppedMaskAction = new QAction(QIcon(":/Qmitk/CreateMask.png"), "Create cropped mask", this);
    createCroppedMaskAction->setEnabled(true);
    QObject::connect(createCroppedMaskAction, SIGNAL(triggered(bool)), this, SLOT(OnCreateCroppedMask(bool)));

    //    QAction* importAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Import...", this );
    //    importAction->setEnabled(true);
    //    QObject::connect( importAction, SIGNAL( triggered(bool) ), this, SLOT( OnImportSegmentationSession(bool) ) );
    //    menu->addAction(importAction);

    menu->addAction(createCroppedMaskAction);

    QSlider *opacitySlider = new QSlider;
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(100);
    opacitySlider->setOrientation(Qt::Horizontal);
    QObject::connect(opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(OnOpacityChanged(int)));

    QLabel *_OpacityLabel = new QLabel("Opacity: ");
    QVBoxLayout *_OpacityWidgetLayout = new QVBoxLayout;
    _OpacityWidgetLayout->setContentsMargins(4, 4, 4, 4);
    _OpacityWidgetLayout->addWidget(_OpacityLabel);
    _OpacityWidgetLayout->addWidget(opacitySlider);
    QWidget *_OpacityWidget = new QWidget;
    _OpacityWidget->setLayout(_OpacityWidgetLayout);

    QWidgetAction *OpacityAction = new QWidgetAction(this);
    OpacityAction->setDefaultWidget(_OpacityWidget);
    //  QObject::connect( m_OpacityAction, SIGNAL( changed() ), this, SLOT( OpacityActionChanged() ) );
    auto workingImage = this->GetWorkingImage();
    auto activeLayer = workingImage->GetActiveLayer();
    auto label = workingImage->GetLabel(pixelValue, activeLayer);

    if (nullptr != label)
    {
      auto opacity = label->GetOpacity();
      opacitySlider->setValue(static_cast<int>(opacity * 100));
    }

    menu->addAction(OpacityAction);
  }
  menu->popup(QCursor::pos());
}

void QmitkLabelSetWidget::OnUnlockAllLabels(bool /*value*/)
{
  GetWorkingImage()->GetActiveLabelSet()->SetAllLabelsLocked(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnLockAllLabels(bool /*value*/)
{
  GetWorkingImage()->GetActiveLabelSet()->SetAllLabelsLocked(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnSetAllLabelsVisible(bool /*value*/)
{
  GetWorkingImage()->GetActiveLabelSet()->SetAllLabelsVisible(true);
  UpdateAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnSetAllLabelsInvisible(bool /*value*/)
{
  GetWorkingImage()->GetActiveLabelSet()->SetAllLabelsVisible(false);
  UpdateAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnSetOnlyActiveLabelVisible(bool /*value*/)
{
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  int pixelValue = GetPixelValueOfSelectedItem();

  workingImage->GetActiveLabelSet()->SetAllLabelsVisible(false);
  workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->SetVisible(true);

  workingImage->GetActiveLabelSet()->UpdateLookupTable(pixelValue);

  this->WaitCursorOn();

  const mitk::Point3D &pos =
    workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->GetCenterOfMassCoordinates();
  this->WaitCursorOff();
  if (pos.GetVnlVector().max_value() > 0.0)
  {
    emit goToLabel(pos);
  }

  UpdateAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnMergeLabel(bool /*value*/)
{
  QmitkSearchLabelDialog dialog(this);
  dialog.setWindowTitle("Select a second label..");
  dialog.SetLabelSuggestionList(GetLabelStringList());
  int dialogReturnValue = dialog.exec();
  if (dialogReturnValue == QDialog::Rejected)
    return;

  int sourcePixelValue = -1;
  for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); i++)
  {
    if (dialog.GetLabelSetWidgetTableCompleteWord() == QString(m_Controls.m_LabelSetTableWidget->item(i, 0)->text()))
      sourcePixelValue = m_Controls.m_LabelSetTableWidget->item(i, 0)->data(Qt::UserRole).toInt();
  }

  if (sourcePixelValue == -1)
  {
    MITK_INFO << "unknown label";
    return;
  }

  int pixelValue = GetPixelValueOfSelectedItem();
  GetWorkingImage()->MergeLabel(pixelValue, sourcePixelValue, GetWorkingImage()->GetActiveLayer());

  UpdateAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnEraseLabel(bool /*value*/)
{
  int pixelValue = GetPixelValueOfSelectedItem();
  QString question = "Do you really want to erase the contents of label \"";
  question.append(
    QString::fromStdString(GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetName()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Erase label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    GetWorkingImage()->EraseLabel(pixelValue);
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetWidget::OnRemoveLabel(bool /*value*/)
{
  int pixelValue = GetPixelValueOfSelectedItem();
  QString question = "Do you really want to remove label \"";
  question.append(
    QString::fromStdString(GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetName()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Remove label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    GetWorkingImage()->GetActiveLabelSet()->RemoveLabel(pixelValue);
    GetWorkingImage()->EraseLabel(pixelValue);
    this->WaitCursorOff();
  }

  ResetAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnRenameLabel(bool /*value*/)
{
  int pixelValue = GetPixelValueOfSelectedItem();
  QmitkNewSegmentationDialog dialog(this);
  dialog.setWindowTitle("Rename Label");
  dialog.SetSuggestionList(m_OrganColors);
  dialog.SetColor(GetWorkingImage()->GetActiveLabelSet()->GetLabel(pixelValue)->GetColor());
  dialog.SetSegmentationName(
    QString::fromStdString(GetWorkingImage()->GetActiveLabelSet()->GetLabel(pixelValue)->GetName()));

  if (dialog.exec() == QDialog::Rejected)
  {
    return;
  }
  QString segmentationName = dialog.GetSegmentationName();
  if (segmentationName.isEmpty())
  {
    segmentationName = "Unnamed";
  }

  GetWorkingImage()->GetActiveLabelSet()->RenameLabel(pixelValue, segmentationName.toStdString(), dialog.GetColor());
  GetWorkingImage()->GetActiveLabelSet()->UpdateLookupTable(pixelValue);

  UpdateAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnCombineAndCreateMask(bool /*value*/)
{
  m_Controls.m_LabelSetTableWidget->selectedRanges();
  // ...to do... //
}

void QmitkLabelSetWidget::OnCreateMasks(bool /*value*/)
{
  m_Controls.m_LabelSetTableWidget->selectedRanges();
  // ..to do.. //
}

void QmitkLabelSetWidget::OnCombineAndCreateSurface(bool /*value*/)
{
  m_Controls.m_LabelSetTableWidget->selectedRanges();
  // ..to do.. //
}

void QmitkLabelSetWidget::OnEraseLabels(bool /*value*/)
{
  QString question = "Do you really want to erase the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Erase selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = m_Controls.m_LabelSetTableWidget->selectedRanges();
    if (ranges.isEmpty())
      return;

    std::vector<mitk::Label::PixelType> VectorOfLablePixelValues;
    foreach (QTableWidgetSelectionRange a, ranges)
      for (int i = a.topRow(); i <= a.bottomRow(); i++)
        VectorOfLablePixelValues.push_back(m_Controls.m_LabelSetTableWidget->item(i, 0)->data(Qt::UserRole).toInt());

    this->WaitCursorOn();
    GetWorkingImage()->EraseLabels(VectorOfLablePixelValues, GetWorkingImage()->GetActiveLayer());
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetWidget::OnRemoveLabels(bool /*value*/)
{
  QString question = "Do you really want to remove selected labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Remove selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = m_Controls.m_LabelSetTableWidget->selectedRanges();
    if (ranges.isEmpty())
    {
      return;
    }

    std::vector<mitk::Label::PixelType> VectorOfLablePixelValues;
    foreach (QTableWidgetSelectionRange a, ranges)
    {
      for (int i = a.topRow(); i <= a.bottomRow(); ++i)
      {
        VectorOfLablePixelValues.push_back(m_Controls.m_LabelSetTableWidget->item(i, 0)->data(Qt::UserRole).toInt());
      }
    }

    this->WaitCursorOn();
    GetWorkingImage()->RemoveLabels(VectorOfLablePixelValues, GetWorkingImage()->GetActiveLayer());
    this->WaitCursorOff();
  }

  ResetAllTableWidgetItems();
}

void QmitkLabelSetWidget::OnMergeLabels(bool /*value*/)
{
  int pixelValue = GetPixelValueOfSelectedItem();
  QString question = "Do you really want to merge selected labels into \"";
  question.append(
    QString::fromStdString(GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetName()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Merge selected label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = m_Controls.m_LabelSetTableWidget->selectedRanges();
    if (ranges.isEmpty())
    {
      return;
    }

    std::vector<mitk::Label::PixelType> vectorOfSourcePixelValues;
    foreach (QTableWidgetSelectionRange a, ranges)
    {
      for (int i = a.topRow(); i <= a.bottomRow(); ++i)
      {
        vectorOfSourcePixelValues.push_back(m_Controls.m_LabelSetTableWidget->item(i, 0)->data(Qt::UserRole).toInt());
      }
    }

    this->WaitCursorOn();
    GetWorkingImage()->MergeLabels(pixelValue, vectorOfSourcePixelValues, GetWorkingImage()->GetActiveLayer());
    this->WaitCursorOff();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetWidget::OnLockedButtonClicked()
{
  int row = -1;
  for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); ++i)
  {
    if (sender() == m_Controls.m_LabelSetTableWidget->cellWidget(i, LOCKED_COL))
    {
      row = i;
    }
  }
  if (row >= 0 && row < m_Controls.m_LabelSetTableWidget->rowCount())
  {
    int pixelValue = m_Controls.m_LabelSetTableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    GetWorkingImage()
      ->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())
      ->SetLocked(!GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetLocked());
  }
}

void QmitkLabelSetWidget::OnVisibleButtonClicked()
{
  int row = -1;
  for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); ++i)
  {
    if (sender() == m_Controls.m_LabelSetTableWidget->cellWidget(i, VISIBLE_COL))
    {
      row = i;
      break;
    }
  }

  if (row >= 0 && row < m_Controls.m_LabelSetTableWidget->rowCount())
  {
    QTableWidgetItem *item = m_Controls.m_LabelSetTableWidget->item(row, 0);
    int pixelValue = item->data(Qt::UserRole).toInt();
    GetWorkingImage()
      ->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())
      ->SetVisible(!GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetVisible());
    GetWorkingImage()->GetActiveLabelSet()->UpdateLookupTable(pixelValue);
  }
}

void QmitkLabelSetWidget::OnColorButtonClicked()
{
  int row = -1;
  for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); ++i)
  {
    if (sender() == m_Controls.m_LabelSetTableWidget->cellWidget(i, COLOR_COL))
    {
      row = i;
    }
  }

  if (row >= 0 && row < m_Controls.m_LabelSetTableWidget->rowCount())
  {
    int pixelValue = m_Controls.m_LabelSetTableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    const mitk::Color &color = GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetColor();
    QColor initial(color.GetRed() * 255, color.GetGreen() * 255, color.GetBlue() * 255);
    QColor qcolor = QColorDialog::getColor(initial, nullptr, QString("Change color"));
    if (!qcolor.isValid())
    {
      return;
    }

    QPushButton *button = static_cast<QPushButton *>(m_Controls.m_LabelSetTableWidget->cellWidget(row, COLOR_COL));
    if (!button)
    {
      return;
    }

    button->setAutoFillBackground(true);

    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(qcolor.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.blue()));
    styleSheet.append("); border: 0;");
    button->setStyleSheet(styleSheet);

    mitk::Color newColor;
    newColor.SetRed(qcolor.red() / 255.0);
    newColor.SetGreen(qcolor.green() / 255.0);
    newColor.SetBlue(qcolor.blue() / 255.0);

    GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->SetColor(newColor);

    GetWorkingImage()->GetActiveLabelSet()->UpdateLookupTable(pixelValue);
  }
}

void QmitkLabelSetWidget::OnRandomColor(bool /*value*/)
{
  int pixelValue = GetPixelValueOfSelectedItem();
  GetWorkingImage()
    ->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())
    ->SetColor(m_ColorSequenceRainbow.GetNextColor());
  GetWorkingImage()->GetActiveLabelSet()->UpdateLookupTable(pixelValue);
  UpdateAllTableWidgetItems();
}

void QmitkLabelSetWidget::SetOrganColors(const QStringList &organColors)
{
  m_OrganColors = organColors;
}

void QmitkLabelSetWidget::OnActiveLabelChanged(int pixelValue)
{
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  assert(workingImage);
  workingImage->GetActiveLabelSet()->SetActiveLabel(pixelValue);
  // MITK_INFO << "Active Label set to << " << pixelValue;

  mitk::SurfaceBasedInterpolationController *interpolator = mitk::SurfaceBasedInterpolationController::GetInstance();
  if (interpolator)
  {
    interpolator->SetActiveLabel(pixelValue);
  }

  workingImage->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnItemClicked(QTableWidgetItem *item)
{
  if (!item)
    return;

  int pixelValue = item->data(Qt::UserRole).toInt();

  QList<QTableWidgetSelectionRange> ranges = m_Controls.m_LabelSetTableWidget->selectedRanges();
  if (!ranges.empty() && ranges.back().rowCount() == 1)
  {
    SelectLabelByPixelValue(pixelValue);
    OnActiveLabelChanged(pixelValue);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetWidget::OnItemDoubleClicked(QTableWidgetItem *item)
{
  if (!item)
    return;

  int pixelValue = item->data(Qt::UserRole).toInt();
  // OnItemClicked(item); <<-- Double click first call OnItemClicked
  WaitCursorOn();
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  workingImage->UpdateCenterOfMass(pixelValue, workingImage->GetActiveLayer());
  const mitk::Point3D &pos =
    workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->GetCenterOfMassCoordinates();
  WaitCursorOff();
  if (pos.GetVnlVector().max_value() > 0.0)
  {
    emit goToLabel(pos);
  }

  workingImage->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::SelectLabelByPixelValue(mitk::Label::PixelType pixelValue)
{
  // MITK_INFO << "QmitkLabelSetWidget::SelectLabelByPixelValue " << pixelValue;

  if (!GetWorkingImage()->ExistLabel(pixelValue))
    return;
  for (int row = 0; row < m_Controls.m_LabelSetTableWidget->rowCount(); row++)
  {
    if (m_Controls.m_LabelSetTableWidget->item(row, 0)->data(Qt::UserRole).toInt() == pixelValue)
    {
      m_Controls.m_LabelSetTableWidget->clearSelection();
      m_Controls.m_LabelSetTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
      m_Controls.m_LabelSetTableWidget->selectRow(row);
      m_Controls.m_LabelSetTableWidget->scrollToItem(m_Controls.m_LabelSetTableWidget->item(row, 0));
      m_Controls.m_LabelSetTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
      // SelectTableWidgetItem(m_Controls.m_LabelSetTableWidget->item(i,0));
      // emit resetView();
      // GetWorkingImage()->Modified();
      return;
    }
  }
}

void QmitkLabelSetWidget::InsertTableWidgetItem(mitk::Label *label)
{
  const mitk::Color &color = label->GetColor();

  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color[0] * 255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[1] * 255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[2] * 255));
  styleSheet.append("); border: 0;");

  QTableWidget *tableWidget = m_Controls.m_LabelSetTableWidget;
  int colWidth = (tableWidget->columnWidth(NAME_COL) < 180) ? 180 : tableWidget->columnWidth(NAME_COL) - 2;
  QString text = fontMetrics().elidedText(label->GetName().c_str(), Qt::ElideMiddle, colWidth);
  QTableWidgetItem *nameItem = new QTableWidgetItem(text);
  nameItem->setTextAlignment(Qt::AlignCenter | Qt::AlignLeft);
  // ---!---
  // IMPORTANT: ADD PIXELVALUE TO TABLEWIDGETITEM.DATA
  nameItem->setData(Qt::UserRole, QVariant(label->GetValue()));
  // ---!---

  QPushButton *pbColor = new QPushButton(tableWidget);
  pbColor->setFixedSize(24, 24);
  pbColor->setCheckable(false);
  pbColor->setAutoFillBackground(true);
  pbColor->setToolTip("Change label color");
  pbColor->setStyleSheet(styleSheet);

  connect(pbColor, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()));

  QString transparentStyleSheet = QLatin1String("background-color: transparent; border: 0;");

  QPushButton *pbLocked = new QPushButton(tableWidget);
  pbLocked->setFixedSize(24, 24);
  QIcon *iconLocked = new QIcon();
  auto lockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg"));
  auto unlockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg"));
  iconLocked->addPixmap(lockIcon.pixmap(64), QIcon::Normal, QIcon::Off);
  iconLocked->addPixmap(unlockIcon.pixmap(64), QIcon::Normal, QIcon::On);
  pbLocked->setIcon(*iconLocked);
  pbLocked->setIconSize(QSize(24, 24));
  pbLocked->setCheckable(true);
  pbLocked->setToolTip("Lock/unlock label");
  pbLocked->setChecked(!label->GetLocked());
  pbLocked->setStyleSheet(transparentStyleSheet);

  connect(pbLocked, SIGNAL(clicked()), this, SLOT(OnLockedButtonClicked()));

  QPushButton *pbVisible = new QPushButton(tableWidget);
  pbVisible->setFixedSize(24, 24);
  pbVisible->setAutoRepeat(false);
  QIcon *iconVisible = new QIcon();
  auto visibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg"));
  auto invisibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg"));
  iconVisible->addPixmap(visibleIcon.pixmap(64), QIcon::Normal, QIcon::Off);
  iconVisible->addPixmap(invisibleIcon.pixmap(64), QIcon::Normal, QIcon::On);
  pbVisible->setIcon(*iconVisible);
  pbVisible->setIconSize(QSize(24, 24));
  pbVisible->setCheckable(true);
  pbVisible->setToolTip("Show/hide label");
  pbVisible->setChecked(!label->GetVisible());
  pbVisible->setStyleSheet(transparentStyleSheet);

  connect(pbVisible, SIGNAL(clicked()), this, SLOT(OnVisibleButtonClicked()));

  int row = tableWidget->rowCount();
  tableWidget->insertRow(row);
  tableWidget->setRowHeight(row, 24);
  tableWidget->setItem(row, 0, nameItem);
  tableWidget->setCellWidget(row, 1, pbLocked);
  tableWidget->setCellWidget(row, 2, pbColor);
  tableWidget->setCellWidget(row, 3, pbVisible);
  tableWidget->selectRow(row);

  // m_LabelSetImage->SetActiveLabel(label->GetPixelValue());
  // m_ToolManager->WorkingDataModified.Send();
  // emit activeLabelChanged(label->GetPixelValue());

  if (row == 0)
  {
    tableWidget->hideRow(row); // hide exterior label
  }
}

void QmitkLabelSetWidget::UpdateAllTableWidgetItems()
{
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  if (!workingImage)
    return;

  // add all labels
  QTableWidget *tableWidget = m_Controls.m_LabelSetTableWidget;
  m_LabelStringList.clear();
  for (int i = 0; i < tableWidget->rowCount(); ++i)
  {
    UpdateTableWidgetItem(tableWidget->item(i, 0));
    m_LabelStringList.append(tableWidget->item(i, 0)->text());
  }

  OnLabelListModified(m_LabelStringList);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::UpdateTableWidgetItem(QTableWidgetItem *item)
{
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  mitk::Label *label = workingImage->GetLabel(item->data(Qt::UserRole).toInt(), workingImage->GetActiveLayer());

  const mitk::Color &color = label->GetColor();

  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color[0] * 255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[1] * 255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[2] * 255));
  styleSheet.append("); border: 0;");

  QTableWidget *tableWidget = m_Controls.m_LabelSetTableWidget;
  int colWidth = (tableWidget->columnWidth(NAME_COL) < 180) ? 180 : tableWidget->columnWidth(NAME_COL) - 2;
  QString text = fontMetrics().elidedText(label->GetName().c_str(), Qt::ElideMiddle, colWidth);
  item->setText(text);

  QPushButton *pbLocked = dynamic_cast<QPushButton *>(tableWidget->cellWidget(item->row(), 1));
  pbLocked->setChecked(!label->GetLocked());

  QPushButton *pbColor = dynamic_cast<QPushButton *>(tableWidget->cellWidget(item->row(), 2));
  pbColor->setStyleSheet(styleSheet);

  QPushButton *pbVisible = dynamic_cast<QPushButton *>(tableWidget->cellWidget(item->row(), 3));
  pbVisible->setChecked(!label->GetVisible());

  if (item->row() == 0)
  {
    tableWidget->hideRow(item->row()); // hide exterior label
  }
}

void QmitkLabelSetWidget::ResetAllTableWidgetItems()
{
  QTableWidget *tableWidget = m_Controls.m_LabelSetTableWidget;
  // remove all rows
  while (tableWidget->rowCount())
  {
    tableWidget->removeRow(0);
  }

  mitk::LabelSetImage *workingImage = GetWorkingImage();
  if (!workingImage)
    return;

  // add all labels
  m_LabelStringList.clear();

  mitk::LabelSet::LabelContainerConstIteratorType it = workingImage->GetActiveLabelSet()->IteratorConstBegin();
  mitk::LabelSet::LabelContainerConstIteratorType end = workingImage->GetActiveLabelSet()->IteratorConstEnd();

  int pixelValue = -1;
  while (it != end)
  {
    InsertTableWidgetItem(it->second);
    if (workingImage->GetActiveLabel() == it->second) // get active
      pixelValue = it->first;
    m_LabelStringList.append(QString(it->second->GetName().c_str()));
    it++;
  }

  SelectLabelByPixelValue(pixelValue);

  OnLabelListModified(m_LabelStringList);

  std::stringstream captionText;
  captionText << "Number of labels: " << workingImage->GetNumberOfLabels(workingImage->GetActiveLayer()) - 1;
  m_Controls.m_lblCaption->setText(captionText.str().c_str());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

int QmitkLabelSetWidget::GetPixelValueOfSelectedItem()
{
  if (m_Controls.m_LabelSetTableWidget->currentItem())
  {
    return m_Controls.m_LabelSetTableWidget->currentItem()->data(Qt::UserRole).toInt();
  }
  return -1;
}

QStringList &QmitkLabelSetWidget::GetLabelStringList()
{
  return m_LabelStringList;
}

void QmitkLabelSetWidget::InitializeTableWidget()
{
  QTableWidget *tableWidged = m_Controls.m_LabelSetTableWidget;

  tableWidged->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  tableWidged->setTabKeyNavigation(false);
  tableWidged->setAlternatingRowColors(false);
  tableWidged->setFocusPolicy(Qt::NoFocus);
  tableWidged->setColumnCount(4);
  tableWidged->resizeColumnToContents(NAME_COL);
  tableWidged->setColumnWidth(LOCKED_COL, 25);
  tableWidged->setColumnWidth(COLOR_COL, 25);
  tableWidged->setColumnWidth(VISIBLE_COL, 25);
  tableWidged->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  tableWidged->setContextMenuPolicy(Qt::CustomContextMenu);
  tableWidged->horizontalHeader()->hide();
  tableWidged->setSortingEnabled(false);
  tableWidged->verticalHeader()->hide();
  tableWidged->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tableWidged->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tableWidged->setSelectionBehavior(QAbstractItemView::SelectRows);

  connect(tableWidged, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(OnItemClicked(QTableWidgetItem *)));
  connect(
    tableWidged, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem *)));
  connect(tableWidged,
          SIGNAL(customContextMenuRequested(const QPoint &)),
          this,
          SLOT(OnTableViewContextMenuRequested(const QPoint &)));

  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(activeLabelChanged(int)), this, SLOT(OnActiveLabelChanged(int))
  // );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(importSegmentation()), this, SLOT( OnImportSegmentation()) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(importLabeledImage()), this, SLOT( OnImportLabeledImage()) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(renameLabel(int, const mitk::Color&, const std::string&)), this,
  // SLOT(OnRenameLabel(int, const mitk::Color&, const std::string&)) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createSurface(int, bool)), this, SLOT(OnCreateSurface(int, bool))
  // );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(toggleOutline(bool)), this, SLOT(OnToggleOutline(bool)) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SIGNAL(goToLabel(const
  // mitk::Point3D&)) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateSurface( const QList<QTableWidgetSelectionRange>&
  // )),
  //    this, SLOT(OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>&)) );

  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createMask(int)), this, SLOT(OnCreateMask(int)) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createCroppedMask(int)), this, SLOT(OnCreateCroppedMask(int)) );
  // connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateMask( const QList<QTableWidgetSelectionRange>&
  // )),
  //    this, SLOT(OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>&)) );
}

void QmitkLabelSetWidget::OnOpacityChanged(int value)
{
  int pixelValue = GetPixelValueOfSelectedItem();
  float opacity = static_cast<float>(value) / 100.0f;
  GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->SetOpacity(opacity);
  GetWorkingImage()->GetActiveLabelSet()->UpdateLookupTable(pixelValue);
}

void QmitkLabelSetWidget::setEnabled(bool enabled)
{
  QWidget::setEnabled(enabled);
  UpdateControls();
}

void QmitkLabelSetWidget::SetDataStorage(mitk::DataStorage *storage)
{
  m_DataStorage = storage;
}

void QmitkLabelSetWidget::OnSearchLabel()
{
  std::string text = m_Controls.m_LabelSearchBox->text().toStdString();
  int pixelValue = -1;
  int row = -1;
  for (int i = 0; i < m_Controls.m_LabelSetTableWidget->rowCount(); ++i)
  {
    if (m_Controls.m_LabelSetTableWidget->item(i, 0)->text().toStdString().compare(text) == 0)
    {
      pixelValue = m_Controls.m_LabelSetTableWidget->item(i, 0)->data(Qt::UserRole).toInt();
      row = i;
      break;
    }
  }
  if (pixelValue == -1)
  {
    return;
  }

  GetWorkingImage()->GetActiveLabelSet()->SetActiveLabel(pixelValue);

  QTableWidgetItem *nameItem = m_Controls.m_LabelSetTableWidget->item(row, NAME_COL);
  if (!nameItem)
  {
    return;
  }

  m_Controls.m_LabelSetTableWidget->clearSelection();
  m_Controls.m_LabelSetTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.m_LabelSetTableWidget->selectRow(row);
  m_Controls.m_LabelSetTableWidget->scrollToItem(nameItem);
  m_Controls.m_LabelSetTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  GetWorkingImage()->GetActiveLabelSet()->SetActiveLabel(pixelValue);

  this->WaitCursorOn();
  mitk::Point3D pos =
    GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetCenterOfMassCoordinates();

  m_ToolManager->WorkingDataChanged();

  if (pos.GetVnlVector().max_value() > 0.0)
  {
    emit goToLabel(pos);
  }
  else
  {
    GetWorkingImage()->UpdateCenterOfMass(pixelValue, GetWorkingImage()->GetActiveLayer());
    mitk::Point3D pos =
      GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetCenterOfMassCoordinates();
    emit goToLabel(pos);
  }

  this->WaitCursorOff();
}

void QmitkLabelSetWidget::OnLabelListModified(const QStringList &list)
{
  QStringListModel *completeModel = static_cast<QStringListModel *>(m_Completer->model());
  completeModel->setStringList(list);
}

mitk::LabelSetImage *QmitkLabelSetWidget::GetWorkingImage()
{
  mitk::DataNode *workingNode = GetWorkingNode();
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);
  return workingImage;
}

mitk::DataNode *QmitkLabelSetWidget::GetWorkingNode()
{
  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  return workingNode;
}

void QmitkLabelSetWidget::UpdateControls()
{
  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  bool hasWorkingData = (workingNode != nullptr);

  m_Controls.m_LabelSetTableWidget->setEnabled(hasWorkingData);
  m_Controls.m_LabelSearchBox->setEnabled(hasWorkingData);

  if (!hasWorkingData)
    return;

  QStringListModel *completeModel = static_cast<QStringListModel *>(m_Completer->model());
  completeModel->setStringList(GetLabelStringList());
}

void QmitkLabelSetWidget::OnCreateCroppedMask(bool)
{
  m_ToolManager->ActivateTool(-1);

  mitk::LabelSetImage *workingImage = GetWorkingImage();
  mitk::Image::Pointer maskImage;
  int pixelValue = GetPixelValueOfSelectedItem();
  try
  {
    this->WaitCursorOn();

    mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
    cropFilter->SetInput(workingImage->CreateLabelMask(pixelValue));
    cropFilter->SetBackgroundValue(0);
    cropFilter->SetMarginFactor(1.15);
    cropFilter->Update();

    maskImage = cropFilter->GetOutput();

    this->WaitCursorOff();
  }
  catch (mitk::Exception &e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  if (maskImage.IsNull())
  {
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  mitk::DataNode::Pointer maskNode = mitk::DataNode::New();
  std::string name = workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->GetName();
  name += "-mask";
  maskNode->SetName(name);
  maskNode->SetData(maskImage);
  maskNode->SetBoolProperty("binary", true);
  maskNode->SetBoolProperty("outline binary", true);
  maskNode->SetBoolProperty("outline binary shadow", true);
  maskNode->SetFloatProperty("outline width", 2.0);
  maskNode->SetColor(workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->GetColor());
  maskNode->SetOpacity(1.0);

  m_DataStorage->Add(maskNode, GetWorkingNode());
}

void QmitkLabelSetWidget::OnCreateMask(bool /*triggered*/)
{
  m_ToolManager->ActivateTool(-1);

  mitk::LabelSetImage *workingImage = GetWorkingImage();
  mitk::Image::Pointer maskImage;
  int pixelValue = GetPixelValueOfSelectedItem();
  try
  {
    this->WaitCursorOn();
    maskImage = workingImage->CreateLabelMask(pixelValue);
    this->WaitCursorOff();
  }
  catch (mitk::Exception &e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  if (maskImage.IsNull())
  {
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  mitk::DataNode::Pointer maskNode = mitk::DataNode::New();
  std::string name = workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->GetName();
  name += "-mask";
  maskNode->SetName(name);
  maskNode->SetData(maskImage);
  maskNode->SetBoolProperty("binary", true);
  maskNode->SetBoolProperty("outline binary", true);
  maskNode->SetBoolProperty("outline binary shadow", true);
  maskNode->SetFloatProperty("outline width", 2.0);
  maskNode->SetColor(workingImage->GetLabel(pixelValue, workingImage->GetActiveLayer())->GetColor());
  maskNode->SetOpacity(1.0);

  m_DataStorage->Add(maskNode, GetWorkingNode());
}

void QmitkLabelSetWidget::OnToggleOutline(bool value)
{
  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  workingNode->SetBoolProperty("labelset.contour.active", value);
  workingNode->GetData()->Modified(); // fixme: workaround to force data-type rendering (and not only property-type)
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnCreateSmoothedSurface(bool /*triggered*/)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode::Pointer workingNode = GetWorkingNode();
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  int pixelValue = GetPixelValueOfSelectedItem();

  mitk::LabelSetImageToSurfaceThreadedFilter::Pointer surfaceFilter = mitk::LabelSetImageToSurfaceThreadedFilter::New();

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer successCommand =
    itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  successCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer errorCommand =
    itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  errorCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

  mitk::DataNode::Pointer groupNode = workingNode;
  surfaceFilter->SetPointerParameter("Group node", groupNode);
  surfaceFilter->SetPointerParameter("Input", workingImage);
  surfaceFilter->SetParameter("RequestedLabel", pixelValue);
  surfaceFilter->SetParameter("Smooth", true);
  surfaceFilter->SetDataStorage(*m_DataStorage);

  mitk::StatusBar::GetInstance()->DisplayText("Surface creation is running in background...");

  try
  {
    surfaceFilter->StartAlgorithm();
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this,
                             "Create Surface",
                             "Could not create a surface mesh out of the selected label. See error log for details.\n");
  }
}

void QmitkLabelSetWidget::OnCreateDetailedSurface(bool /*triggered*/)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode::Pointer workingNode = GetWorkingNode();
  mitk::LabelSetImage *workingImage = GetWorkingImage();
  int pixelValue = GetPixelValueOfSelectedItem();

  mitk::LabelSetImageToSurfaceThreadedFilter::Pointer surfaceFilter = mitk::LabelSetImageToSurfaceThreadedFilter::New();

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer successCommand =
    itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  successCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer errorCommand =
    itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  errorCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

  mitk::DataNode::Pointer groupNode = workingNode;
  surfaceFilter->SetPointerParameter("Group node", groupNode);
  surfaceFilter->SetPointerParameter("Input", workingImage);
  surfaceFilter->SetParameter("RequestedLabel", pixelValue);
  surfaceFilter->SetParameter("Smooth", false);
  surfaceFilter->SetDataStorage(*m_DataStorage);

  mitk::StatusBar::GetInstance()->DisplayText("Surface creation is running in background...");

  try
  {
    surfaceFilter->StartAlgorithm();
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this,
                             "Create Surface",
                             "Could not create a surface mesh out of the selected label. See error log for details.\n");
  }
}

void QmitkLabelSetWidget::OnImportLabeledImage()
{
  /*
    m_ToolManager->ActivateTool(-1);

    mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
    assert(referenceNode);

    // Ask the user for a list of files to open
    QStringList fileNames = QFileDialog::getOpenFileNames( this, "Open Image", m_LastFileOpenPath,
                                                          mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

    if (fileNames.empty())
      return;

    try
    {
      this->WaitCursorOn();
      mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>( fileNames.front().toStdString() );
      if (image.IsNull())
      {
        this->WaitCursorOff();
        QMessageBox::information(this, "Import Labeled Image", "Could not load the selected segmentation.\n");
        return;
      }

      mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
      newImage->InitializeByLabeledImage(image);
      this->WaitCursorOff();

      mitk::DataNode::Pointer newNode = mitk::DataNode::New();
      std::string newName = referenceNode->GetName();
      newName += "-labels";
      newNode->SetName(newName);
      newNode->SetData(newImage);
      m_DataStorage->Add(newNode, referenceNode);
    }
    catch (mitk::Exception & e)
    {
      this->WaitCursorOff();
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      QMessageBox::information(this, "Import Labeled Image", "Could not load the selected segmentation. See error log
    for details.\n");
      return;
     }

    this->UpdateControls();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    */
}

void QmitkLabelSetWidget::OnImportSegmentation()
{
  /*
    m_ToolManager->ActivateTool(-1);

    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
    assert(workingNode);

    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
    assert(workingImage);

    std::string fileExtensions("Segmentation files (*.lset);;");
    QString qfileName = QFileDialog::getOpenFileName(this, "Import Segmentation", m_LastFileOpenPath,
    fileExtensions.c_str() );
    if (qfileName.isEmpty() ) return;

    mitk::NrrdLabelSetImageReader::Pointer reader = mitk::NrrdLabelSetImageReader::New();
    reader->SetFileName(qfileName.toLatin1());

    try
    {
      this->WaitCursorOn();
      reader->Update();
      mitk::LabelSetImage::Pointer newImage = reader->GetOutput();
      workingImage->Concatenate(newImage);
      this->WaitCursorOff();
    }
    catch ( mitk::Exception& e )
    {
      this->WaitCursorOff();
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      QMessageBox::information(this, "Import Segmentation", "Could not import the selected segmentation session.\n See
    error log for details.\n");
    }
  */
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkLabelSetWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

void QmitkLabelSetWidget::OnThreadedCalculationDone()
{
  mitk::StatusBar::GetInstance()->Clear();
}
