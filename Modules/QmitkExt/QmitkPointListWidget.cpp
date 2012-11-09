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
#include "QmitkPointListWidget.h"
#include <mitkGlobalInteraction.h>
#include <mitkPointSetReader.h>
#include <mitkPointSetWriter.h>

#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include <QmitkEditPointDialog.h>

#include "btnLoad.xpm"
#include "btnSave.xpm"
#include "btnClear.xpm"
#include "btnSetPoints.xpm"
#include "btnSetPointsManually.xpm"
#include "btnUp.xpm"
#include "btnDown.xpm"



QmitkPointListWidget::QmitkPointListWidget(QWidget *parent, int orientation):
  QWidget(parent), m_PointListView(NULL),  m_MultiWidget(NULL),  m_PointSetNode(NULL), m_Orientation(0),  m_MovePointUpBtn(NULL),
  m_MovePointDownBtn(NULL), m_RemovePointBtn(NULL), m_SavePointsBtn(NULL), m_LoadPointsBtn(NULL), m_ToggleAddPoint(NULL),
  m_AddPoint(NULL), m_Interactor(NULL), m_TimeStep(0), m_EditAllowed(true), m_NodeObserverTag(0),
  m_Snc1(NULL),
  m_Snc2(NULL),
  m_Snc3(NULL)
{
  m_PointListView = new QmitkPointListView();

  if(orientation != 0)
    m_Orientation = orientation;

  SetupUi();
  SetupConnections();
  ObserveNewNode(NULL);

}

QmitkPointListWidget::~QmitkPointListWidget()
{
  if (m_Interactor)
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
  m_Interactor = NULL;

  if(m_PointSetNode && m_NodeObserverTag)
  {
    m_PointSetNode->RemoveObserver(m_NodeObserverTag);
    m_NodeObserverTag = 0;
  }

  m_MultiWidget = NULL;
  delete m_PointListView;
}


void QmitkPointListWidget::SetupConnections()
{
  //m_PointListView->setModel(m_PointListModel);

  connect(this->m_LoadPointsBtn, SIGNAL(clicked()), this, SLOT(OnBtnLoadPoints()));
  connect(this->m_SavePointsBtn, SIGNAL(clicked()), this, SLOT(OnBtnSavePoints()));
  connect(this->m_MovePointUpBtn, SIGNAL(clicked()), this, SLOT(MoveSelectedPointUp()));
  connect(this->m_MovePointDownBtn, SIGNAL(clicked()), this, SLOT(MoveSelectedPointDown()));
  connect(this->m_RemovePointBtn, SIGNAL(clicked()), this, SLOT(RemoveSelectedPoint()));
  connect(this->m_ToggleAddPoint, SIGNAL(toggled(bool)), this, SLOT(OnBtnAddPoint(bool)));
  connect(this->m_AddPoint, SIGNAL(clicked()), this, SLOT(OnBtnAddPointManually()));
  connect(this->m_PointListView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(OnListDoubleClick()));
  connect(this->m_PointListView, SIGNAL(SignalPointSelectionChanged()), this, SLOT(OnPointSelectionChanged()));
}

void QmitkPointListWidget::SetupUi()
{
  //Setup the buttons

  m_ToggleAddPoint = new QPushButton();//iconSetPoints, "", this);
  m_ToggleAddPoint->setMaximumSize(25,25);
  m_ToggleAddPoint->setCheckable(true);
  m_ToggleAddPoint->setToolTip("Toggle point editing (use SHIFT  + Left Mouse Button to add Points)");
  QIcon iconAdd(btnSetPoints_xpm);
  m_ToggleAddPoint->setIcon(iconAdd);

  m_AddPoint = new QPushButton();//iconSetPoints, "", this);
  m_AddPoint->setMaximumSize(25,25);
  m_AddPoint->setToolTip("Manually add point");
  QIcon iconAddManually(btnSetPointsManually_xpm);
  m_AddPoint->setIcon(iconAddManually);

  m_RemovePointBtn = new QPushButton();
  m_RemovePointBtn->setMaximumSize(25, 25);
  const QIcon iconDel(btnClear_xpm);
  m_RemovePointBtn->setIcon(iconDel);
  m_RemovePointBtn->setToolTip("Erase one point from list   (Hotkey: DEL)");

  m_MovePointUpBtn = new QPushButton();
  m_MovePointUpBtn->setMaximumSize(25, 25);
  const QIcon iconUp(btnUp_xpm);
  m_MovePointUpBtn->setIcon(iconUp);
  m_MovePointUpBtn->setToolTip("Swap selected point upwards   (Hotkey: F2)");

  m_MovePointDownBtn = new QPushButton();
  m_MovePointDownBtn->setMaximumSize(25, 25);
  const QIcon iconDown(btnDown_xpm);
  m_MovePointDownBtn->setIcon(iconDown);
  m_MovePointDownBtn->setToolTip("Swap selected point downwards   (Hotkey: F3)");

  m_SavePointsBtn = new QPushButton();
  m_SavePointsBtn->setMaximumSize(25, 25);
  QIcon iconSave(btnSave_xpm);
  m_SavePointsBtn->setIcon(iconSave);
  m_SavePointsBtn->setToolTip("Save points to file");

  m_LoadPointsBtn = new QPushButton();
  m_LoadPointsBtn->setMaximumSize(25, 25);
  QIcon iconLoad(btnLoad_xpm);
  m_LoadPointsBtn->setIcon(iconLoad);
  m_LoadPointsBtn->setToolTip("Load list of points from file (REPLACES current content)");


  int i;

  QBoxLayout* lay1;
  QBoxLayout* lay2;

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
    i=-1;
    break;

  case 2:
    lay1 = new QHBoxLayout(this);
    lay2 = new QVBoxLayout();
    i=0;
    break;

  default:
    lay1 = new QVBoxLayout(this);
    lay2 = new QHBoxLayout();
    i=-1;
    break;

  }

  //setup Layouts

  this->setLayout(lay1);
  lay1->addLayout(lay2);

  lay2->stretch(true);
  lay2->addWidget(m_ToggleAddPoint);
  lay2->addWidget(m_AddPoint);
  lay2->addWidget(m_RemovePointBtn);
  lay2->addWidget(m_MovePointUpBtn);
  lay2->addWidget(m_MovePointDownBtn);
  lay2->addWidget(m_SavePointsBtn);
  lay2->addWidget(m_LoadPointsBtn);


  //lay2->addSpacing();;

  lay1->insertWidget(i,m_PointListView);
  this->setLayout(lay1);
}

void QmitkPointListWidget::SetPointSet(mitk::PointSet* newPs)
{
  if(newPs == NULL)
    return;

  this->m_PointSetNode->SetData(newPs);
  dynamic_cast<QmitkPointListModel*>(this->m_PointListView->model())->SetPointSetNode(m_PointSetNode);
  ObserveNewNode(m_PointSetNode);
}

void QmitkPointListWidget::SetPointSetNode(mitk::DataNode *newNode)
{
  ObserveNewNode(newNode);
  dynamic_cast<QmitkPointListModel*>(this->m_PointListView->model())->SetPointSetNode(newNode);
}

void QmitkPointListWidget::OnBtnSavePoints()
{
  if ((dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData()))  == NULL)
    return; // don't write empty point sets. If application logic requires something else then do something else.
  if ((dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData()))->GetSize() == 0)
    return;

  // let the user choose a file
  std::string name("");

  QString fileNameProposal = QString("/PointSet.mps");//.arg(m_PointSetNode->GetName().c_str()); //"PointSet.mps";
  QString aFilename = QFileDialog::getSaveFileName( NULL, "Save point set", QDir::currentPath() + fileNameProposal, "MITK Pointset (*.mps)" );
  if ( aFilename.isEmpty() )
    return;

  try
  {
    // instantiate the writer and add the point-sets to write
    mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
    writer->SetInput( dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData()) );
    writer->SetFileName( aFilename.toLatin1() );
    writer->Update();
  }
  catch(...)
  {
    QMessageBox::warning( this, "Save point set",
                          QString("File writer reported problems writing %1\n\n"
                                  "PLEASE CHECK output file!").arg(aFilename) );
  }
}

void QmitkPointListWidget::OnBtnLoadPoints()
{
  // get the name of the file to load
  QString filename = QFileDialog::getOpenFileName( NULL, "Open MITK Pointset", "", "MITK Point Sets (*.mps)");
  if ( filename.isEmpty() ) return;

  // attempt to load file
  try
  {
    mitk::PointSetReader::Pointer reader = mitk::PointSetReader::New();
    reader->SetFileName( filename.toLatin1() );
    reader->Update();

    mitk::PointSet::Pointer pointSet = reader->GetOutput();
    if ( pointSet.IsNull() )
    {
      QMessageBox::warning( this, "Load point set", QString("File reader could not read %1").arg(filename) );
      return;
    }

    // loading successful

    //      bool interactionOn( m_Interactor.IsNotNull() );
    //      if (interactionOn)
    //      {
    //        OnEditPointSetButtonToggled(false);
    //      }
    //
    this->SetPointSet(pointSet);
    //      if (interactionOn)
    //      {
    //        OnEditPointSetButtonToggled(true);
    //      }
  }
  catch(...)
  {
    QMessageBox::warning( this, "Load point set", QString("File reader collapsed while reading %1").arg(filename) );
  }
  emit PointListChanged();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::PointSet* QmitkPointListWidget::GetPointSet()
{
  return dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());
}

mitk::DataNode* QmitkPointListWidget::GetPointSetNode()
{
  return m_PointSetNode;
}

void QmitkPointListWidget::SetMultiWidget(QmitkStdMultiWidget *multiWidget)
{
  this->m_MultiWidget = multiWidget;
  m_PointListView->SetMultiWidget(multiWidget);
}

void QmitkPointListWidget::RemoveSelectedPoint()
{

  if (!m_PointSetNode) return;
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>( m_PointSetNode->GetData() );
  if (!pointSet) return;
  if (pointSet->GetSize() == 0) return;

  QmitkPointListModel* pointListModel = dynamic_cast<QmitkPointListModel*>( m_PointListView->model() );
  pointListModel->RemoveSelectedPoint();
  emit PointListChanged();
}

void QmitkPointListWidget::MoveSelectedPointDown()
{

  if (!m_PointSetNode) return;
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>( m_PointSetNode->GetData() );
  if (!pointSet) return;
  if (pointSet->GetSize() == 0) return;

  QmitkPointListModel* pointListModel = dynamic_cast<QmitkPointListModel*>( m_PointListView->model() );
  pointListModel->MoveSelectedPointDown();
  emit PointListChanged();
}

void QmitkPointListWidget::MoveSelectedPointUp()
{

  if (!m_PointSetNode) return;
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>( m_PointSetNode->GetData() );
  if (!pointSet) return;
  if (pointSet->GetSize() == 0) return;

  QmitkPointListModel* pointListModel = dynamic_cast<QmitkPointListModel*>( m_PointListView->model() );
  pointListModel->MoveSelectedPointUp();
  emit PointListChanged();
}

void QmitkPointListWidget::OnBtnAddPoint(bool checked)
{
  if (m_PointSetNode)
  {
    if (checked)
    {
      m_Interactor = dynamic_cast<mitk::PointSetInteractor*>(m_PointSetNode->GetInteractor());

      if (m_Interactor.IsNull())//if not present, instanciate one
        m_Interactor = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode);

      //add it to global interaction to activate it
      mitk::GlobalInteraction::GetInstance()->AddInteractor( m_Interactor );
    }
    else if ( m_Interactor )
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
      m_Interactor = NULL;
    }
    emit EditPointSets(checked);
  }
}

void QmitkPointListWidget::OnBtnAddPointManually()
{
  mitk::PointSet* pointSet = this->GetPointSet();
  int currentPosition = pointSet->GetSize();
  QmitkEditPointDialog editPointDialog(this);
  editPointDialog.SetPoint(pointSet, currentPosition, m_TimeStep);
  editPointDialog.exec();
}

void QmitkPointListWidget::OnListDoubleClick()
{
  ;
}

void QmitkPointListWidget::OnPointSelectionChanged()
{
  emit this->PointSelectionChanged();
}

void QmitkPointListWidget::DeactivateInteractor(bool /*deactivate*/)
{
  ;
}

void QmitkPointListWidget::EnableEditButton( bool enabled )
{
  m_EditAllowed = enabled;
  if (enabled == false)
    m_ToggleAddPoint->setEnabled(false);
  else
    m_ToggleAddPoint->setEnabled(true);
  OnBtnAddPoint(enabled);
}


void QmitkPointListWidget::ObserveNewNode( mitk::DataNode* node )
{
  // remove old observer
  if ( m_PointSetNode )
  {
    if (m_Interactor)
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
      m_Interactor = NULL;
      m_ToggleAddPoint->setChecked( false );
    }

    m_PointSetNode->RemoveObserver( m_NodeObserverTag );
    m_NodeObserverTag = 0;
  }

  m_PointSetNode = node;

  // add new observer if necessary
  if ( m_PointSetNode )
  {
    itk::ReceptorMemberCommand<QmitkPointListWidget>::Pointer command = itk::ReceptorMemberCommand<QmitkPointListWidget>::New();
    command->SetCallbackFunction( this, &QmitkPointListWidget::OnNodeDeleted );
    m_NodeObserverTag = m_PointSetNode->AddObserver( itk::DeleteEvent(), command );
  }
  else
  {
    m_NodeObserverTag = 0;
  }

  if (m_EditAllowed == true)
    m_ToggleAddPoint->setEnabled( m_PointSetNode );
  else
    m_ToggleAddPoint->setEnabled( false );

  m_RemovePointBtn->setEnabled( m_PointSetNode );
  m_LoadPointsBtn->setEnabled( m_PointSetNode );
  m_SavePointsBtn->setEnabled(m_PointSetNode);
  m_AddPoint->setEnabled(m_PointSetNode);
}

void QmitkPointListWidget::OnNodeDeleted( const itk::EventObject &  /*e*/ )
{
  if(m_PointSetNode.IsNotNull() && ! m_NodeObserverTag)
    m_PointSetNode->RemoveObserver( m_NodeObserverTag );
  m_NodeObserverTag = 0;
  m_PointSetNode = NULL;
  m_PointListView->SetPointSetNode(NULL);
  m_ToggleAddPoint->setEnabled(false);

  m_RemovePointBtn->setEnabled( false );
  m_LoadPointsBtn->setEnabled( false );
  m_SavePointsBtn->setEnabled(false);
  m_AddPoint->setEnabled(false);
}


void QmitkPointListWidget::SetSnc1(mitk::SliceNavigationController* snc)
{
   m_Snc1 = snc;
   m_PointListView->SetSnc1(snc);
}

void QmitkPointListWidget::SetSnc2(mitk::SliceNavigationController* snc)
{
   m_Snc2 = snc;
   m_PointListView->SetSnc2(snc);
}

void QmitkPointListWidget::SetSnc3(mitk::SliceNavigationController* snc)
{
   m_Snc3 = snc;
   m_PointListView->SetSnc3(snc);
}

void QmitkPointListWidget::UnselectEditButton()
{
  m_ToggleAddPoint->setChecked(false);
}