/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPointListWidget.h"

#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <mitkIOUtil.h>

#include <QmitkAbstractMultiWidget.h>
#include <QmitkEditPointDialog.h>
#include <QmitkStyleManager.h>

#include <mitkPointSetDataInteractor.h>

QmitkPointListWidget::QmitkPointListWidget(QWidget *parent, int orientation)
  : QWidget(parent),
    m_PointListView(nullptr),
    m_PointSetNode(nullptr),
    m_Orientation(0),
    m_MovePointUpBtn(nullptr),
    m_MovePointDownBtn(nullptr),
    m_RemovePointBtn(nullptr),
    m_SavePointsBtn(nullptr),
    m_LoadPointsBtn(nullptr),
    m_ToggleAddPoint(nullptr),
    m_AddPoint(nullptr),
    m_TimeStepDisplay(nullptr),
    m_DataInteractor(nullptr),
    m_TimeStep(0),
    m_EditAllowed(true),
    m_NodeObserverTag(0)
{
  m_PointListView = new QmitkPointListView();

  if (orientation != 0)
    m_Orientation = orientation;

  SetupUi();
  SetupConnections();
  ObserveNewNode(nullptr);
}

QmitkPointListWidget::~QmitkPointListWidget()
{
  m_DataInteractor = nullptr;

  if (m_PointSetNode && m_NodeObserverTag)
  {
    m_PointSetNode->RemoveObserver(m_NodeObserverTag);
    m_NodeObserverTag = 0;
  }

  delete m_PointListView;
}

void QmitkPointListWidget::SetupConnections()
{
  connect(this->m_LoadPointsBtn, SIGNAL(clicked()), this, SLOT(OnBtnLoadPoints()));
  connect(this->m_SavePointsBtn, SIGNAL(clicked()), this, SLOT(OnBtnSavePoints()));
  connect(this->m_MovePointUpBtn, SIGNAL(clicked()), this, SLOT(MoveSelectedPointUp()));
  connect(this->m_MovePointDownBtn, SIGNAL(clicked()), this, SLOT(MoveSelectedPointDown()));
  connect(this->m_RemovePointBtn, SIGNAL(clicked()), this, SLOT(RemoveSelectedPoint()));
  connect(this->m_ToggleAddPoint, SIGNAL(toggled(bool)), this, SLOT(OnBtnAddPoint(bool)));
  connect(this->m_AddPoint, SIGNAL(clicked()), this, SLOT(OnBtnAddPointManually()));
  connect(this->m_PointListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnListDoubleClick()));
  connect(this->m_PointListView, SIGNAL(SignalPointSelectionChanged()), this, SLOT(OnPointSelectionChanged()));
  connect(this->m_PointListView, SIGNAL(SignalTimeStepChanged(int)), this, SLOT(OnTimeStepChanged(int)));
}

void QmitkPointListWidget::OnTimeStepChanged(int timeStep)
{
  m_TimeStepLabel->setText(QString("%1").arg(timeStep));
}

void QmitkPointListWidget::SetupUi()
{
  // Setup the buttons

  m_ToggleAddPoint = new QPushButton();
  m_ToggleAddPoint->setMaximumSize(25, 25);
  m_ToggleAddPoint->setCheckable(true);
  m_ToggleAddPoint->setToolTip("Toggle point editing (use SHIFT  + Left Mouse Button to add Points)");
  m_ToggleAddPoint->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/plus.svg")));

  m_AddPoint = new QPushButton();
  m_AddPoint->setMaximumSize(25, 25);
  m_AddPoint->setToolTip("Manually add point");
  m_AddPoint->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/plus-xyz.svg")));

  m_RemovePointBtn = new QPushButton();
  m_RemovePointBtn->setMaximumSize(25, 25);
  m_RemovePointBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/eraser.svg")));
  m_RemovePointBtn->setToolTip("Erase one point from list   (Hotkey: DEL)");

  m_MovePointUpBtn = new QPushButton();
  m_MovePointUpBtn->setMaximumSize(25, 25);
  m_MovePointUpBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/arrow-up.svg")));
  m_MovePointUpBtn->setToolTip("Swap selected point upwards   (Hotkey: F2)");

  m_MovePointDownBtn = new QPushButton();
  m_MovePointDownBtn->setMaximumSize(25, 25);
  m_MovePointDownBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/arrow-down.svg")));
  m_MovePointDownBtn->setToolTip("Swap selected point downwards   (Hotkey: F3)");

  m_SavePointsBtn = new QPushButton();
  m_SavePointsBtn->setMaximumSize(25, 25);
  m_SavePointsBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/save.svg")));
  m_SavePointsBtn->setToolTip("Save points to file");

  m_LoadPointsBtn = new QPushButton();
  m_LoadPointsBtn->setMaximumSize(25, 25);
  m_LoadPointsBtn->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/QtWidgetsExt/folder-open.svg")));
  m_LoadPointsBtn->setToolTip("Load list of points from file (REPLACES current content)");

  int i;

  QBoxLayout *lay1;
  QBoxLayout *lay2;
  QBoxLayout *lay3;

  switch (m_Orientation)
  {
    case 0:
      lay1 = new QVBoxLayout(this);
      lay2 = new QHBoxLayout();
      i = 0;
      break;

    case 1:
      lay1 = new QHBoxLayout(this);
      lay2 = new QVBoxLayout();
      i = -1;
      break;

    case 2:
      lay1 = new QHBoxLayout(this);
      lay2 = new QVBoxLayout();
      i = 0;
      break;

    default:
      lay1 = new QVBoxLayout(this);
      lay2 = new QHBoxLayout();
      i = -1;
      break;
  }

  // setup Layouts

  this->setLayout(lay1);
  lay2->stretch(true);
  lay2->addWidget(m_ToggleAddPoint);
  lay2->addWidget(m_AddPoint);
  lay2->addWidget(m_RemovePointBtn);
  lay2->addWidget(m_MovePointUpBtn);
  lay2->addWidget(m_MovePointDownBtn);
  lay2->addWidget(m_SavePointsBtn);
  lay2->addWidget(m_LoadPointsBtn);

  // setup Labels
  m_TimeStepDisplay = new QLabel;
  m_TimeStepLabel = new QLabel;
  lay3 = new QHBoxLayout;

  m_TimeStepDisplay->setMaximumSize(200, 15);

  lay3->stretch(true);
  lay3->setAlignment(Qt::AlignLeft);
  lay3->addWidget(m_TimeStepDisplay);
  lay3->addWidget(m_TimeStepLabel);

  m_TimeStepDisplay->setText("Time Step: ");
  m_TimeStepLabel->setMaximumSize(10, 15);

  this->OnTimeStepChanged(0);

  //Add Layouts

  lay1->insertWidget(i, m_PointListView);
  this->setLayout(lay1);
  lay1->addLayout(lay2);
  lay1->addLayout(lay3);
}

void QmitkPointListWidget::SetPointSet(mitk::PointSet *newPs)
{
  if (newPs == nullptr)
    return;

  this->m_PointSetNode->SetData(newPs);
  dynamic_cast<QmitkPointListModel *>(this->m_PointListView->model())->SetPointSetNode(m_PointSetNode);
  ObserveNewNode(m_PointSetNode);
}

void QmitkPointListWidget::SetPointSetNode(mitk::DataNode *newNode)
{
  if (m_DataInteractor.IsNotNull())
    m_DataInteractor->SetDataNode(newNode);

  ObserveNewNode(newNode);
  dynamic_cast<QmitkPointListModel *>(this->m_PointListView->model())->SetPointSetNode(newNode);
}

void QmitkPointListWidget::OnBtnSavePoints()
{
  if ((dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData())) == nullptr)
    return; // don't write empty point sets. If application logic requires something else then do something else.
  if ((dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData()))->GetSize() == 0)
    return;

  // take the previously defined name of node as proposal for filename
  std::string nodeName = m_PointSetNode->GetName();
  nodeName = "/" + nodeName + ".mps";
  QString fileNameProposal = QString();
  fileNameProposal.append(nodeName.c_str());

  QString aFilename = QFileDialog::getSaveFileName(
    nullptr, "Save point set", QDir::currentPath() + fileNameProposal, "MITK Pointset (*.mps)");
  if (aFilename.isEmpty())
    return;

  try
  {
    mitk::IOUtil::Save(m_PointSetNode->GetData(), aFilename.toStdString());
  }
  catch (...)
  {
    QMessageBox::warning(this,
                         "Save point set",
                         QString("File writer reported problems writing %1\n\n"
                                 "PLEASE CHECK output file!")
                           .arg(aFilename));
  }
}

void QmitkPointListWidget::OnBtnLoadPoints()
{
  // get the name of the file to load
  QString filename = QFileDialog::getOpenFileName(nullptr, "Open MITK Pointset", "", "MITK Point Sets (*.mps)");
  if (filename.isEmpty())
    return;

  // attempt to load file
  try
  {
    mitk::PointSet::Pointer pointSet = mitk::IOUtil::Load<mitk::PointSet>(filename.toStdString());
    if (pointSet.IsNull())
    {
      QMessageBox::warning(this, "Load point set", QString("File reader could not read %1").arg(filename));
      return;
    }

    // loading successful

    this->SetPointSet(pointSet);
  }
  catch (...)
  {
    QMessageBox::warning(this, "Load point set", QString("File reader collapsed while reading %1").arg(filename));
  }
  emit PointListChanged();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::PointSet *QmitkPointListWidget::GetPointSet()
{
  return dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData());
}

mitk::DataNode *QmitkPointListWidget::GetPointSetNode()
{
  return m_PointSetNode;
}

void QmitkPointListWidget::SetMultiWidget(QmitkAbstractMultiWidget* multiWidget)
{
  m_PointListView->SetMultiWidget(multiWidget);
}

void QmitkPointListWidget::RemoveSelectedPoint()
{
  if (!m_PointSetNode)
    return;
  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData());
  if (!pointSet)
    return;
  if (pointSet->GetSize() == 0)
    return;

  QmitkPointListModel *pointListModel = dynamic_cast<QmitkPointListModel *>(m_PointListView->model());
  pointListModel->RemoveSelectedPoint();
  emit PointListChanged();
}

void QmitkPointListWidget::MoveSelectedPointDown()
{
  if (!m_PointSetNode)
    return;
  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData());
  if (!pointSet)
    return;
  if (pointSet->GetSize() == 0)
    return;

  QmitkPointListModel *pointListModel = dynamic_cast<QmitkPointListModel *>(m_PointListView->model());
  pointListModel->MoveSelectedPointDown();
  emit PointListChanged();
}

void QmitkPointListWidget::MoveSelectedPointUp()
{
  if (!m_PointSetNode)
    return;
  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData());
  if (!pointSet)
    return;
  if (pointSet->GetSize() == 0)
    return;

  QmitkPointListModel *pointListModel = dynamic_cast<QmitkPointListModel *>(m_PointListView->model());
  pointListModel->MoveSelectedPointUp();
  emit PointListChanged();
}

void QmitkPointListWidget::OnBtnAddPoint(bool checked)
{
  if (m_PointSetNode.IsNotNull())
  {
    if (checked)
    {
      m_DataInteractor = m_PointSetNode->GetDataInteractor();
      // If no data Interactor is present create a new one
      if (m_DataInteractor.IsNull())
      {
        // Create PointSetData Interactor
        m_DataInteractor = mitk::PointSetDataInteractor::New();
        // Load the according state machine for regular point set interaction
        m_DataInteractor->LoadStateMachine("PointSet.xml");
        // Set the configuration file that defines the triggers for the transitions
        m_DataInteractor->SetEventConfig("PointSetConfig.xml");
        // set the DataNode (which already is added to the DataStorage
        m_DataInteractor->SetDataNode(m_PointSetNode);
      }
    }
    else
    {
      m_PointSetNode->SetDataInteractor(nullptr);
      m_DataInteractor = nullptr;
    }
    emit EditPointSets(checked);
  }
}

void QmitkPointListWidget::OnBtnAddPointManually()
{
  mitk::PointSet *pointSet = this->GetPointSet();
  QmitkEditPointDialog editPointDialog(this);

  if (this->GetPointSet()->IsEmpty())
  {
      editPointDialog.SetPoint(pointSet, 0, m_TimeStep);
  }

  else
  {
      mitk::PointSet::PointsIterator maxIt = pointSet->GetMaxId();
      mitk::PointSet::PointIdentifier maxId = maxIt->Index();
      editPointDialog.SetPoint(pointSet, maxId + 1, m_TimeStep);
  }

  editPointDialog.exec();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPointListWidget::OnListDoubleClick()
{
}

void QmitkPointListWidget::OnPointSelectionChanged()
{
  emit this->PointSelectionChanged();
}

void QmitkPointListWidget::DeactivateInteractor(bool)
{
}

void QmitkPointListWidget::EnableEditButton(bool enabled)
{
  m_EditAllowed = enabled;
  if (enabled == false)
    m_ToggleAddPoint->setEnabled(false);
  else
    m_ToggleAddPoint->setEnabled(true);
  OnBtnAddPoint(enabled);
}

void QmitkPointListWidget::ObserveNewNode(mitk::DataNode *node)
{
  if (m_DataInteractor.IsNotNull())
    m_DataInteractor->SetDataNode(node);

  // remove old observer
  if (m_PointSetNode)
  {
    if (m_DataInteractor)
    {
      m_DataInteractor = nullptr;
      m_ToggleAddPoint->setChecked(false);
    }

    m_PointSetNode->RemoveObserver(m_NodeObserverTag);
    m_NodeObserverTag = 0;
  }

  m_PointSetNode = node;
  // add new observer if necessary
  if (m_PointSetNode)
  {
    itk::ReceptorMemberCommand<QmitkPointListWidget>::Pointer command =
      itk::ReceptorMemberCommand<QmitkPointListWidget>::New();
    command->SetCallbackFunction(this, &QmitkPointListWidget::OnNodeDeleted);
    m_NodeObserverTag = m_PointSetNode->AddObserver(itk::DeleteEvent(), command);
  }
  else
  {
    m_NodeObserverTag = 0;
  }

  if (m_EditAllowed == true)
    m_ToggleAddPoint->setEnabled(m_PointSetNode);
  else
    m_ToggleAddPoint->setEnabled(false);

  m_RemovePointBtn->setEnabled(m_PointSetNode);
  m_LoadPointsBtn->setEnabled(m_PointSetNode);
  m_SavePointsBtn->setEnabled(m_PointSetNode);
  m_AddPoint->setEnabled(m_PointSetNode);
}

void QmitkPointListWidget::OnNodeDeleted(const itk::EventObject &)
{
  if (m_PointSetNode.IsNotNull() && !m_NodeObserverTag)
    m_PointSetNode->RemoveObserver(m_NodeObserverTag);
  m_NodeObserverTag = 0;
  m_PointSetNode = nullptr;
  m_PointListView->SetPointSetNode(nullptr);
  m_ToggleAddPoint->setEnabled(false);

  m_RemovePointBtn->setEnabled(false);
  m_LoadPointsBtn->setEnabled(false);
  m_SavePointsBtn->setEnabled(false);
  m_AddPoint->setEnabled(false);
}

void QmitkPointListWidget::AddSliceNavigationController(mitk::SliceNavigationController *snc)
{
  m_PointListView->AddSliceNavigationController(snc);
}

void QmitkPointListWidget::RemoveSliceNavigationController(mitk::SliceNavigationController *snc)
{
  m_PointListView->RemoveSliceNavigationController(snc);
}

void QmitkPointListWidget::UnselectEditButton()
{
  m_ToggleAddPoint->setChecked(false);
}
