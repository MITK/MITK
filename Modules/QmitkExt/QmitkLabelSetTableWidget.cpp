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

#include "QmitkLabelSetTableWidget.h"

#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkLabelSetImage.h>

#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <QColorDialog>
#include <QMessageBox>
#include <QWidgetAction>
#include <QLabel>
#include <QTableWidgetSelectionRange>
#include <QVBoxLayout>
#include <QStringList>
#include <QApplication>

#include <itkCommand.h>

QmitkLabelSetTableWidget::QmitkLabelSetTableWidget( QWidget* parent ) : QTableWidget(parent),
m_BlockEvents(false),
m_AutoSelectNewLabels(true),
m_AllVisible(true),
m_AllLocked(false),
m_AllOutlined(false),
m_ColorSequenceRainbow(mitk::ColorSequenceRainbow::New())
{
  m_ColorSequenceRainbow->GoToBegin();
  this->Initialize();
}

QmitkLabelSetTableWidget::~QmitkLabelSetTableWidget()
{
  // remove listeners
  if(this->m_LabelSetImage.IsNotNull())
  {
    this->m_LabelSetImage->AddLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelAdded ) );

    this->m_LabelSetImage->RemoveLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelRemoved ) );

    this->m_LabelSetImage->ModifyLabelEvent.RemoveListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::LabelModified ) );

    this->m_LabelSetImage->AllLabelsModifiedEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::AllLabelsModified ) );
  }
}

void QmitkLabelSetTableWidget::setEnabled(bool enabled)
{
  if (!enabled)
  {
    this->SetActiveLabelSetImage(NULL);
  }

  QWidget::setEnabled(enabled);
}

void QmitkLabelSetTableWidget::Initialize()
{
  connect(this, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(OnItemClicked(QTableWidgetItem *)));
  connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem *)));
//  connect(this, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(OnItemNameChanged(QTableWidgetItem *)));

  this->setTabKeyNavigation(false);
  this->setAlternatingRowColors(false);
  this->setFocusPolicy( Qt::NoFocus );
  this->setColumnCount(4);
  this->setColumnWidth(NAME_COL,180);
  this->setColumnWidth(LOCKED_COL,25);
  this->setColumnWidth(COLOR_COL,25);
  this->setColumnWidth(VISIBLE_COL,25);
  this->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
  this->setStyleSheet("QTableView {selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #3257bf, stop: 1 white);}");
//  this->setStyleSheet("QTableView {selection-background-color: #5151cd; selection-color: #ffffff;}");
 // this->setStyleSheet("background: rgb(255,255,255);color:rgb(0,0,0); font-family:Arial Narrow;font-size:18px;"
//"selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #3257bf, stop: 1 white);");

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  connect( this, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );

  this->horizontalHeader()->hide();
  this->setSortingEnabled ( false );
  //this->horizontalHeader()->hide();
  this->verticalHeader()->hide();
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);

  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectRows);
}

const QStringList& QmitkLabelSetTableWidget::GetLabelList()
{
  return m_LabelStringList;
}

bool QmitkLabelSetTableWidget::GetAutoSelectNewLabels()
{
  return m_AutoSelectNewLabels;
}

void QmitkLabelSetTableWidget::SetActiveLabelSetImage(mitk::LabelSetImage* _image)
{
  // reset only if labelset node really changed
  if (m_LabelSetImage != _image)
  {
    // if there is an old labelset, remove listeners
    if(m_LabelSetImage.IsNotNull())
    {
      this->m_LabelSetImage->AddLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
          this, &QmitkLabelSetTableWidget::LabelAdded ) );

      this->m_LabelSetImage->RemoveLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
          this, &QmitkLabelSetTableWidget::LabelRemoved ) );

      this->m_LabelSetImage->ModifyLabelEvent.RemoveListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
        , int>( this, &QmitkLabelSetTableWidget::LabelModified ) );

      this->m_LabelSetImage->AllLabelsModifiedEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
          this, &QmitkLabelSetTableWidget::AllLabelsModified ) );
    }

    m_LabelSetImage = _image;

    // add listeners to the new labelset
    if(m_LabelSetImage.IsNotNull())
    {
      this->m_LabelSetImage->AddLabelEvent.AddListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
          this, &QmitkLabelSetTableWidget::LabelAdded ) );

      this->m_LabelSetImage->RemoveLabelEvent.AddListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
          this, &QmitkLabelSetTableWidget::LabelRemoved ) );

      this->m_LabelSetImage->ModifyLabelEvent.AddListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
        , int>( this, &QmitkLabelSetTableWidget::LabelModified ) );

      this->m_LabelSetImage->AllLabelsModifiedEvent.AddListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
          this, &QmitkLabelSetTableWidget::AllLabelsModified ) );
    }

    this->Reset();
  }
}

void QmitkLabelSetTableWidget::SetActiveLabel(int index)
{
  if (index >= 0 && index < this->rowCount())
  {
    this->m_LabelSetImage->SetActiveLabel(index,true);
  }
}

void QmitkLabelSetTableWidget::SetActiveLabel(const std::string& text)
{
  this->SetActiveLabel(m_LabelStringList.indexOf(text.c_str()));
  this->BusyCursorOn();
  const mitk::Point3D& pos = m_LabelSetImage->GetLabelCenterOfMassCoordinates(m_LabelSetImage->GetActiveLabelIndex(), true);
  this->BusyCursorOff();
  if (pos.GetVnlVector().max_value() > 0.0)
      emit goToLabel(pos);
}

void QmitkLabelSetTableWidget::LabelAdded()
{
  // this is an event function, avoid calling ourself
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;

    this->InsertItem();

    m_BlockEvents = false;
  }
}

void QmitkLabelSetTableWidget::LabelRemoved( )
{
  // this is an event function, avoid calling ourself
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;

    this->Reset();

    m_BlockEvents = false;
  }
}

void QmitkLabelSetTableWidget::AllLabelsModified( )
{
  // this is an event function, avoid calling ourself
  for(int i=0; i<this->rowCount(); i++)
    this->LabelModified(i);
}

void QmitkLabelSetTableWidget::LabelModified( int index )
{
  // this is an event function, avoid calling ourself
  QTableWidgetItem* nameItem = this->item(index,NAME_COL);

  if (nameItem)
  {
    int colWidth = (this->columnWidth(NAME_COL) < 180) ? 180 : this->columnWidth(NAME_COL)-2;
    QString text = fontMetrics().elidedText(m_LabelSetImage->GetLabelName(index).c_str(), Qt::ElideMiddle, colWidth);
  //  QString text = fontMetrics().elidedText(m_LabelSetImage->GetLabelName(index).c_str(), Qt::ElideMiddle, this->columnWidth(NAME_COL)-2);
    nameItem->setText(text);
    nameItem->setToolTip(text);
  }

  QPushButton* lbutton = (QPushButton*) this->cellWidget(index,LOCKED_COL);
  if (lbutton)
  {
    lbutton->setChecked(!m_LabelSetImage->GetLabelLocked(index));
  }

  QPushButton* button = (QPushButton*) this->cellWidget(index,COLOR_COL);
  if (button)
  {
    const mitk::Color& color = m_LabelSetImage->GetLabelColor(index);
    button->setAutoFillBackground(true);
    QColor qcolor(color[0]*255,color[1]*255,color[2]*255);
    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(qcolor.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.blue()));
    styleSheet.append(")");
    button->setStyleSheet(styleSheet);
  }

  QPushButton* vbutton = (QPushButton*) this->cellWidget(index,VISIBLE_COL);
  if (vbutton)
  {
    vbutton->setChecked(!m_LabelSetImage->GetLabelVisible(index));
  }

  this->clearSelection();
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->selectRow(m_LabelSetImage->GetActiveLabelIndex());
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void QmitkLabelSetTableWidget::SetAutoSelectNewItems( bool value )
{
  m_AutoSelectNewLabels = value;
}

void QmitkLabelSetTableWidget::OnColorButtonClicked()
{
  int row;
  for(int i=0; i<this->rowCount(); i++)
  {
    if (sender() == this->cellWidget(i,COLOR_COL))
    {
      row = i;
    }
  }

  if (row >= 0 && row < this->rowCount())
  {
    const mitk::Color& color = m_LabelSetImage->GetLabelColor(row);
    QColor initial(color.GetRed()*255,color.GetGreen()*255,color.GetBlue()*255);
    QColor qcolor = QColorDialog::getColor(initial,0,QString("Change color"));
    if (!qcolor.isValid())
    return;

    QPushButton* button = (QPushButton*) this->cellWidget(row,COLOR_COL);
    if (!button) return;

    button->setAutoFillBackground(true);

    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(qcolor.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.blue()));
    styleSheet.append(")");
    button->setStyleSheet(styleSheet);

    mitk::Color newColor;
    newColor.SetRed(qcolor.red()/255.0);
    newColor.SetGreen(qcolor.green()/255.0);
    newColor.SetBlue(qcolor.blue()/255.0);

    m_LabelSetImage->SetLabelColor(row,newColor);
  }
}

void QmitkLabelSetTableWidget::OnLockedButtonClicked()
{
  int row;
  for(int i=0; i<this->rowCount(); i++)
  {
    if (sender() == this->cellWidget(i,LOCKED_COL))
    {
      row = i;
    }
  }

  if (row >= 0 && row < this->rowCount())
  {
  m_LabelSetImage->SetLabelLocked( row, !m_LabelSetImage->GetLabelLocked(row) );
  }
}

void QmitkLabelSetTableWidget::OnVisibleButtonClicked()
{
  int row;
  for(int i=0; i<this->rowCount(); i++)
  {
    if (sender() == this->cellWidget(i,VISIBLE_COL))
    {
      row = i;
    }
  }

  if (row >= 0 && row < this->rowCount())
  {
   m_LabelSetImage->SetLabelVisible(row, !m_LabelSetImage->GetLabelVisible(row) );
   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnItemClicked(QTableWidgetItem *item)
{
  if (!item) return;
  int row = item->row();

  if (row >= 0 && row < this->rowCount())
    m_LabelSetImage->SetActiveLabel(row, false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnItemDoubleClicked(QTableWidgetItem *item)
{
  if (!item) return;
  int row = item->row();

  if (row >= 0 && row < this->rowCount())
  {
    m_LabelSetImage->SetActiveLabel(row,false);
    this->BusyCursorOn();
    const mitk::Point3D& pos = m_LabelSetImage->GetLabelCenterOfMassCoordinates(m_LabelSetImage->GetActiveLabelIndex(), true);
    this->BusyCursorOff();
    if (pos.GetVnlVector().max_value() > 0.0)
        emit goToLabel(pos);
  }
}

void QmitkLabelSetTableWidget::SetAllLabelsVisible(bool value)
{
  m_AllVisible = !value;
  this->OnSetAllLabelsVisible(true);
}

void QmitkLabelSetTableWidget::InsertItem()
{
  int index = this->rowCount();

  m_LabelStringList.append( QString::fromStdString(m_LabelSetImage->GetLabelName(index)) );

  const mitk::Color& color = m_LabelSetImage->GetLabelColor(index);

  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color[0]*255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[1]*255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[2]*255));
  styleSheet.append(")");

  int colWidth = (this->columnWidth(NAME_COL) < 180) ? 180 : this->columnWidth(NAME_COL)-2;
  QString text = fontMetrics().elidedText(m_LabelSetImage->GetLabelName(index).c_str(), Qt::ElideMiddle, colWidth);
  QTableWidgetItem *nameItem = new QTableWidgetItem(text);
  nameItem->setToolTip(m_LabelSetImage->GetLabelName(index).c_str());
  nameItem->setTextAlignment(Qt::AlignCenter | Qt::AlignLeft);

  QPushButton * pbColor = new QPushButton(this);
  pbColor->setFixedSize(24,24);
//  pbColor->setFlat(true);
  pbColor->setCheckable(false);
  pbColor->setAutoFillBackground(true);
  pbColor->setToolTip(QString("Change label color"));
  pbColor->setStyleSheet(styleSheet);
//  pbColor->setEnabled(!isBackground);

  connect( pbColor, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()) );

  QPushButton * pbLocked = new QPushButton(this);
  pbLocked->setFixedSize(24,24);
//  pbLocked->setFlat(true);
  QIcon * iconLocked = new QIcon();
  iconLocked->addFile(QString::fromUtf8(":/QmitkExt/lock.png"), QSize(), QIcon::Normal, QIcon::Off);
  iconLocked->addFile(QString::fromUtf8(":/QmitkExt/unlock.png"), QSize(), QIcon::Normal, QIcon::On);
  pbLocked->setIcon(*iconLocked);
  pbLocked->setIconSize(QSize(24,24));
  pbLocked->setCheckable(true);
  pbLocked->setToolTip(QString("Lock/unlock label"));
  pbLocked->setChecked(!m_LabelSetImage->GetLabelLocked(index));
  //pbLocked->setEnabled(!isBackground);
  connect( pbLocked, SIGNAL(clicked()), this, SLOT(OnLockedButtonClicked()) );

  QPushButton * pbVisible = new QPushButton(this);
  pbVisible->setFixedSize(24,24);
//  pbVisible->setFlat(true);
  pbVisible->setAutoRepeat(false);
  QIcon * iconVisible = new QIcon();
  iconVisible->addFile(QString::fromUtf8(":/QmitkExt/visible.png"), QSize(), QIcon::Normal, QIcon::Off);
  iconVisible->addFile(QString::fromUtf8(":/QmitkExt/invisible.png"), QSize(), QIcon::Normal, QIcon::On);
  pbVisible->setIcon(*iconVisible);
  pbVisible->setIconSize(QSize(24,24));
  pbVisible->setCheckable(true);
  pbVisible->setToolTip(QString("Show/hide label"));
//  pbVisible->setEnabled(!isBackground);
  pbVisible->setChecked(!m_LabelSetImage->GetLabelVisible(index));

  connect( pbVisible, SIGNAL(clicked()), this, SLOT(OnVisibleButtonClicked()) );

  int row = this->rowCount();
  this->insertRow(row);
  this->setRowHeight(row,24);
//  this->setItem(row, 0, idItem );
  this->setItem(row, 0, nameItem );
  this->setCellWidget(row, 1, pbLocked);
  this->setCellWidget(row, 2, pbColor);
  this->setCellWidget(row, 3, pbVisible);

  if (m_AutoSelectNewLabels)
  {
    this->selectRow(row);
    m_LabelSetImage->SetActiveLabel(row,true);
  }

  emit labelListModified(m_LabelStringList);
}

void QmitkLabelSetTableWidget::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  QTableWidgetItem *itemAt = this->itemAt(pos);
  if (!itemAt) return;
  int row = itemAt->row();
  QMenu* menu = new QMenu(this);

  if (this->selectedItems().size()>1)
  {
    QAction* mergeAction = new QAction(QIcon(":/QmitkExt/mergelabels.png"), "Merge selection on current label", this );
    mergeAction->setEnabled(true);
    QObject::connect( mergeAction, SIGNAL( triggered(bool) ), this, SLOT( OnMergeLabels(bool) ) );
    menu->addAction(mergeAction);

    QAction* removeLabelsAction = new QAction(QIcon(":/QmitkExt/removelabel.png"), "Remove selected labels", this );
    removeLabelsAction->setEnabled(true);
    QObject::connect( removeLabelsAction, SIGNAL( triggered(bool) ), this, SLOT( OnRemoveLabels(bool) ) );
    menu->addAction(removeLabelsAction);

    QAction* eraseLabelsAction = new QAction(QIcon(":/QmitkExt/eraselabel.png"), "Erase selected labels", this );
    eraseLabelsAction->setEnabled(true);
    QObject::connect( eraseLabelsAction, SIGNAL( triggered(bool) ), this, SLOT( OnEraseLabels(bool) ) );
    menu->addAction(eraseLabelsAction);

    QAction* smoothLabelsAction = new QAction(QIcon(":/QmitkExt/smoothlabel.png"), "Smooth selected labels", this );
    smoothLabelsAction->setEnabled(true);
    QObject::connect( smoothLabelsAction, SIGNAL( triggered(bool) ), this, SLOT( OnEraseLabels(bool) ) );
    menu->addAction(smoothLabelsAction);

    QAction* createSurfacesAction = new QAction(QIcon(":/QmitkExt/createsurface.png"), "Create a surface for each selected label", this );
    createSurfacesAction->setEnabled(true);
    QObject::connect( createSurfacesAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateSurfaces(bool) ) );
    menu->addAction(createSurfacesAction);

    QAction* combineAndCreateSurfaceAction = new QAction(QIcon(":/QmitkExt/createsurface.png"), "Combine and create a surface", this );
    combineAndCreateSurfaceAction->setEnabled(true);
    QObject::connect( combineAndCreateSurfaceAction, SIGNAL( triggered(bool) ), this, SLOT( OnCombineAndCreateSurface(bool) ) );
    menu->addAction(combineAndCreateSurfaceAction);

    QAction* createMasksAction = new QAction(QIcon(":/QmitkExt/createmask.png"), "Create a mask for each selected label", this );
    createMasksAction->setEnabled(true);
    QObject::connect( createMasksAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateMasks(bool) ) );
    menu->addAction(createMasksAction);

    QAction* combineAndCreateMaskAction = new QAction(QIcon(":/QmitkExt/createmask.png"), "Combine and create a mask", this );
    combineAndCreateMaskAction->setEnabled(true);
    QObject::connect( combineAndCreateMaskAction, SIGNAL( triggered(bool) ), this, SLOT( OnCombineAndCreateMask(bool) ) );
    menu->addAction(combineAndCreateMaskAction);
  }
  else
  {
    QAction* toggleOutlineAction = new QAction(QIcon(":/QmitkExt/outline.png"), "Outline/Filled all", this );
    toggleOutlineAction->setEnabled(true);
    QObject::connect( toggleOutlineAction, SIGNAL( triggered(bool) ), this, SLOT( OnToggleOutline(bool) ) );
    menu->addAction(toggleOutlineAction);

    QAction* renameAction = new QAction(QIcon(":/QmitkExt/renamelabel.png"), "Rename...", this );
    renameAction->setEnabled(true);
    QObject::connect( renameAction, SIGNAL( triggered(bool) ), this, SLOT( OnRenameLabel(bool) ) );
    menu->addAction(renameAction);

    QAction* removeAction = new QAction(QIcon(":/QmitkExt/removelabel.png"), "Remove label", this );
    removeAction->setEnabled(true);
    QObject::connect( removeAction, SIGNAL( triggered(bool) ), this, SLOT( OnRemoveLabel(bool) ) );
    menu->addAction(removeAction);

    QAction* eraseAction = new QAction(QIcon(":/QmitkExt/eraselabel.png"), "Erase label", this );
    eraseAction->setEnabled(true);
    QObject::connect( eraseAction, SIGNAL( triggered(bool) ), this, SLOT( OnEraseLabel(bool) ) );
    menu->addAction(eraseAction);

    QAction* smoothAction = new QAction(QIcon(":/QmitkExt/smoothlabel.png"), "Smooth label", this );
    smoothAction->setEnabled(true);
    QObject::connect( smoothAction, SIGNAL( triggered(bool) ), this, SLOT( OnSmoothLabel(bool) ) );
    menu->addAction(smoothAction);

    QAction* randomColorAction = new QAction(QIcon(":/QmitkExt/randomcolor.png"), "Random color", this );
    randomColorAction->setEnabled(true);
    QObject::connect( randomColorAction, SIGNAL( triggered(bool) ), this, SLOT( OnRandomColor(bool) ) );
    menu->addAction(randomColorAction);

    QAction* viewOnlyAction = new QAction(QIcon(":/QmitkExt/visible.png"), "View only", this );
    viewOnlyAction->setEnabled(true);
    QObject::connect( viewOnlyAction, SIGNAL( triggered(bool) ), this, SLOT( OnSetOnlyActiveLabelVisible(bool) ) );
    menu->addAction(viewOnlyAction);

    QAction* viewAllAction = new QAction(QIcon(":/QmitkExt/visible.png"), "View/Hide all", this );
    viewAllAction->setEnabled(true);
    QObject::connect( viewAllAction, SIGNAL( triggered(bool) ), this, SLOT( OnSetAllLabelsVisible(bool) ) );
    menu->addAction(viewAllAction);

    QAction* lockAllAction = new QAction(QIcon(":/QmitkExt/lock.png"), "Lock/Unlock all", this );
    lockAllAction->setEnabled(true);
    QObject::connect( lockAllAction, SIGNAL( triggered(bool) ), this, SLOT( OnLockAllLabels(bool) ) );
    menu->addAction(lockAllAction);

    QAction* createSurfaceAction = new QAction(QIcon(":/QmitkExt/createsurface.png"), "Create surface", this );
    createSurfaceAction->setEnabled(true);
    QObject::connect( createSurfaceAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateSurface(bool) ) );

    menu->addAction(createSurfaceAction);

    QAction* createMaskAction = new QAction(QIcon(":/QmitkExt/createmask.png"), "Create mask", this );
    createMaskAction->setEnabled(true);
    QObject::connect( createMaskAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateMask(bool) ) );

    menu->addAction(createMaskAction);

    m_OpacitySlider = new QSlider;
    m_OpacitySlider->setMinimum(0);
    m_OpacitySlider->setMaximum(100);
    m_OpacitySlider->setOrientation(Qt::Horizontal);
    QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) ), this, SLOT( OpacityChanged(int) ) );

    QLabel* _OpacityLabel = new QLabel("Opacity: ");
    QVBoxLayout* _OpacityWidgetLayout = new QVBoxLayout;
    _OpacityWidgetLayout->setContentsMargins(4,4,4,4);
    _OpacityWidgetLayout->addWidget(_OpacityLabel);
    _OpacityWidgetLayout->addWidget(m_OpacitySlider);
    QWidget* _OpacityWidget = new QWidget;
    _OpacityWidget->setLayout(_OpacityWidgetLayout);

    m_OpacityAction = new QWidgetAction(this);
    m_OpacityAction->setDefaultWidget(_OpacityWidget);
  //  QObject::connect( m_OpacityAction, SIGNAL( changed() ), this, SLOT( OpacityActionChanged() ) );
    m_OpacitySlider->setValue(static_cast<int>(m_LabelSetImage->GetLabelOpacity(row)*100));

    menu->addAction(m_OpacityAction);
  }
  menu->popup(QCursor::pos());
}

void QmitkLabelSetTableWidget::OpacityChanged(int value)
{
  float opacity = static_cast<float>(value)/100.0f;
  m_LabelSetImage->SetLabelOpacity(this->currentRow(),opacity);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnRemoveLabel(bool value)
{
  QString question = "Do you really want to remove label \"";
  question.append(QString::fromStdString(this->m_LabelSetImage->GetLabelName(this->currentRow()).c_str()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Remove label",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->BusyCursorOn();
    this->m_LabelSetImage->RemoveLabel( this->currentRow() );
    this->BusyCursorOff();
  }
}

void QmitkLabelSetTableWidget::OnToggleOutline(bool value)
{
  m_AllOutlined = !m_AllOutlined;
  emit toggleOutline(m_AllOutlined);
}

void QmitkLabelSetTableWidget::OnMergeLabels(bool value)
{
  QString question = "Do you really want to merge selected labels into \"";
  question.append(QString::fromStdString(this->m_LabelSetImage->GetLabelName(this->currentRow()).c_str()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Merge selected label",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
      return;

    std::vector<int> indexes;
    for (int i=0; i<ranges.size(); i++)
    {
      int begin = ranges.at(i).topRow();
      for (int j=0; j<ranges.at(i).rowCount(); j++)
      {
        if (begin+j != this->currentRow())
        {
          indexes.push_back(begin+j);
        }
      }
    }

    this->BusyCursorOn();
    this->m_LabelSetImage->MergeLabels(indexes,this->currentRow());
    this->BusyCursorOff();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}


void QmitkLabelSetTableWidget::OnRemoveLabels(bool value)
{
  QString question = "Do you really want to remove selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Remove selected labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
      return;

    std::vector<int> indexes;
    for (int i=0; i<ranges.size(); i++)
    {
      int begin = ranges.at(i).topRow();
      for (int j=0; j<ranges.at(i).rowCount(); j++)
      {
          indexes.push_back(begin+j);
      }
    }

    this->BusyCursorOn();
    this->m_LabelSetImage->RemoveLabels(indexes);
    this->BusyCursorOff();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkLabelSetTableWidget::OnSmoothLabels(bool value)
{
  QString question = "Do you really want to smooth the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Smooth selected labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
    return;

    std::vector<int> indexes;
    for (int i=0; i<ranges.size(); i++)
    {
        int begin = ranges.at(i).topRow();
        for (int j=0; j<ranges.at(i).rowCount(); j++)
        {
            indexes.push_back(begin+j);
        }
    }

    this->BusyCursorOn();
    this->m_LabelSetImage->SmoothLabels(indexes);
    this->BusyCursorOff();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnSmoothLabel(bool value)
{
  this->BusyCursorOn();
  this->m_LabelSetImage->SmoothLabel( this->currentRow() );
  this->BusyCursorOff();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnEraseLabels(bool value)
{
  QString question = "Do you really want to erase the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Erase selected labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
    return;

    std::vector<int> indexes;
    for (int i=0; i<ranges.size(); i++)
    {
        int begin = ranges.at(i).topRow();
        for (int j=0; j<ranges.at(i).rowCount(); j++)
        {
            //this->m_LabelSetImage->SetLabelSelected(begin+j, true);
            indexes.push_back(begin+j);
        }
    }

    this->BusyCursorOn();
    this->m_LabelSetImage->EraseLabels(indexes);
    this->BusyCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnEraseLabel(bool value)
{
  QString question = "Do you really want to erase the contents of label \"";
  question.append(QString::fromStdString(this->m_LabelSetImage->GetLabelName(this->currentRow()).c_str()));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Erase label",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->BusyCursorOn();
    this->m_LabelSetImage->EraseLabel( this->currentRow(), false );
    this->BusyCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnNewLabel(bool value)
{
  emit newLabel();
}

void QmitkLabelSetTableWidget::OnCreateSurface(bool value)
{
  emit createSurface( this->currentRow() );
}

void QmitkLabelSetTableWidget::OnCreateSurfaces(bool value)
{
  //emit createSurfaces( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnCombineAndCreateSurface(bool value)
{
  emit combineAndCreateSurface( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnCreateMask(bool value)
{
  emit createMask( this->currentRow() );
}

void QmitkLabelSetTableWidget::OnCreateMasks(bool value)
{
//   emit createMasks( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnCombineAndCreateMask(bool value)
{
  emit combineAndCreateMask( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnLockAllLabels(bool value)
{
  m_AllLocked = !m_AllLocked;
  this->m_LabelSetImage->SetAllLabelsLocked(m_AllLocked);
}

void QmitkLabelSetTableWidget::OnSetAllLabelsVisible(bool value)
{
  m_AllVisible = !m_AllVisible;
  this->m_LabelSetImage->SetAllLabelsVisible(m_AllVisible);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnRemoveAllLabels(bool value)
{
  QString question = "Do you really want to remove all labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Remove all labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->m_LabelSetImage->RemoveAllLabels();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnSetOnlyActiveLabelVisible(bool value)
{
  m_AllVisible = false;
  this->m_LabelSetImage->SetAllLabelsVisible(m_AllVisible);
  this->m_LabelSetImage->SetLabelVisible( this->currentRow(), true);
  this->BusyCursorOn();
  const mitk::Point3D& pos = m_LabelSetImage->GetLabelCenterOfMassCoordinates(m_LabelSetImage->GetActiveLabelIndex(), true);
  this->BusyCursorOff();
  if (pos.GetVnlVector().max_value() > 0.0)
    emit goToLabel(pos);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnRenameLabel(bool value)
{
  int index = this->currentRow();
  emit renameLabel( index, this->m_LabelSetImage->GetLabelColor(index), this->m_LabelSetImage->GetLabelName(index) );
}

void QmitkLabelSetTableWidget::OnRandomColor(bool value)
{
  this->m_LabelSetImage->SetLabelColor( this->currentRow(), this->m_ColorSequenceRainbow->GetNextColor() );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::Reset()
{
  // remove all rows
  while (this->rowCount())
    this->removeRow( 0 );

  m_LabelStringList.clear();

  if (this->m_LabelSetImage.IsNotNull()) // sometimes Reset() is called without a labelset image
  {
    // add all labels
    int counter = 0;
    while (counter != this->m_LabelSetImage->GetNumberOfLabels())
    {
      this->InsertItem();
      ++counter;
    }
  }
}

void QmitkLabelSetTableWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkLabelSetTableWidget::BusyCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
}

void QmitkLabelSetTableWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetTableWidget::BusyCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetTableWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}
