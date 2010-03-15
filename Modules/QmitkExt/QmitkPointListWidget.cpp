/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkPointListWidget.h"

#include "mitkGlobalInteraction.h"
#include "mitkPointSetReader.h"
#include "mitkPointSetWriter.h"
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"

#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>

#include "btnSetPoints.xpm"
#include "btnClear.xpm"
#include "btnLoad.xpm"
#include "btnSave.xpm"
    
QmitkPointListWidget::QmitkPointListWidget( QWidget* parent, Qt::WindowFlags f )
:QWidget( parent, f ), m_ListView(NULL), m_BtnEdit(NULL), m_BtnClear(NULL), m_BtnLoad(NULL),
 m_BtnSave(NULL), m_PointSetNode(NULL), m_Interactor(NULL), m_EditAllowed(true),
 m_NodeObserverTag(0)
{
  QGridLayout* layout = new QGridLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
    
  m_ListView = new QmitkPointListView;
  layout->addWidget(m_ListView, 0, 0, 1, 1);
  m_ListView->setToolTip("Use F2 to move the selected point up, use F3 to move the selected point down, double click on a point toggles editing");
  connect( m_ListView, SIGNAL(PointSelectionChanged()), this, SIGNAL(PointSelectionChanged()) );  // forward signal from list view
  
  
  QWidget* buttonWidget = new QWidget;
  layout->addWidget( buttonWidget, 1, 0, 1, 1 );

  QHBoxLayout* buttonLayout = new QHBoxLayout;
  buttonWidget->setLayout( buttonLayout );
  buttonLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

  buttonLayout->setMargin(0);

  const QIcon iconEdit( btnSetPoints_xpm ); // installs/removes pointset interactor
  m_BtnEdit = new QPushButton( iconEdit, "", this );
  m_BtnEdit->setToolTip("Toggle point editing (use SHIFT  + Left Mouse Button to add Points)");
  connect( m_BtnEdit, SIGNAL(toggled(bool)), this, SLOT(OnEditPointSetButtonToggled(bool)) ); 
  m_BtnEdit->setCheckable( true ); 
  buttonLayout->addWidget( m_BtnEdit );

  const QIcon iconClear( btnClear_xpm ); // clears whole point set
  m_BtnClear = new QPushButton( iconClear, "", this );
  m_BtnClear->setToolTip("Erase all points from list. Use the delete key to delete the selected point.");
  connect( m_BtnClear, SIGNAL(clicked()), this, SLOT(OnClearPointSetButtonClicked()) );
  buttonLayout->addWidget(m_BtnClear);

  const QIcon iconLoad( btnLoad_xpm ); // loads a point set from file
  m_BtnLoad = new QPushButton( iconLoad, "", this );
  m_BtnLoad->setToolTip("Load list of points from file (REPLACES current content)");
  connect( m_BtnLoad, SIGNAL(clicked()), this, SLOT(OnLoadPointSetButtonClicked()) );
  buttonLayout->addWidget(m_BtnLoad);

  const QIcon iconSave( btnSave_xpm ); // saves point set to file
  m_BtnSave = new QPushButton( iconSave, "", this );
  m_BtnSave->setToolTip("Save points to file");
  connect( m_BtnSave, SIGNAL(clicked()), this, SLOT(OnSavePointSetButtonClicked()) );
  buttonLayout->addWidget(m_BtnSave);

  layout->setRowStretch(0, 10);
  layout->setRowStretch(1, 1);
  this->setLayout( layout );

  ObserveNewNode(NULL);
}


QmitkPointListWidget::~QmitkPointListWidget()
{ 
  if (m_Interactor)
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
  m_Interactor = NULL;
  if (m_PointSetNode != NULL)
    m_PointSetNode->RemoveObserver( m_NodeObserverTag );
  m_PointSetNode = NULL;
}


void QmitkPointListWidget::SetPointSetNode( mitk::DataNode* node )
{
  ObserveNewNode(node);

  if (node)
  {
    mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>( node->GetData() );
    m_ListView->SetPointSet( pointSet );
  }
  else
  {
    m_ListView->SetPointSet( NULL );
  }
}


void QmitkPointListWidget::SetMultiWidget( QmitkStdMultiWidget* multiWidget )
{
  m_ListView->SetMultiWidget( multiWidget );
}


void QmitkPointListWidget::DeactivateInteractor(bool deactivate)
{
  if (deactivate)
  {
    if (m_Interactor)
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
      m_Interactor = NULL;
      m_BtnEdit->setChecked( false );
    }
  }
}
   

void QmitkPointListWidget::ObserveNewNode( mitk::DataNode* node )
{
  // remove old observer
  if ( m_PointSetNode != NULL )
  {
    if (m_Interactor)
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
      m_Interactor = NULL;
      m_BtnEdit->setChecked( false );
    }
   
    m_PointSetNode->RemoveObserver( m_NodeObserverTag );
  }

  m_PointSetNode = node;

  // add new observer if necessary
  if ( m_PointSetNode != NULL )
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
    m_BtnEdit->setEnabled( m_PointSetNode != NULL );
  else
    m_BtnEdit->setEnabled( false ); 

  m_BtnClear->setEnabled( m_PointSetNode != NULL );
  m_BtnLoad->setEnabled( m_PointSetNode != NULL );
  m_BtnSave->setEnabled( m_PointSetNode != NULL );
}


void QmitkPointListWidget::OnNodeDeleted( const itk::EventObject &  /*e*/ )
{
  m_PointSetNode = NULL;
  m_NodeObserverTag = 0;
  m_ListView->SetPointSet( NULL );
}


void QmitkPointListWidget::OnEditPointSetButtonToggled(bool checked)
{
  if (m_PointSetNode)
  {
    if (checked)
    {
      m_Interactor = mitk::PointSetInteractor::New("pointsetinteractor", m_PointSetNode);
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


void QmitkPointListWidget::OnClearPointSetButtonClicked()
{
  if (!m_PointSetNode) return;
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>( m_PointSetNode->GetData() );
  if (!pointSet) return;
  if (pointSet->GetSize() == 0) return; // we don't have to disturb by asking silly questions

  switch( QMessageBox::question( this, tr("Clear Points"),
          tr("Remove all points from the displayed list?"),
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) 
  {
    case QMessageBox::Yes: 
      if (pointSet)
      {
        mitk::PointSet::DataType::PointsContainer::Pointer pointsContainer = pointSet->GetPointSet()->GetPoints();
        pointsContainer->Initialize(); // a call to initialize results in clearing the points container

        //dummy call to update PointSetVtkMapper3D (bug fix oder better bug work around #2436)
        mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, pointSet->GetPoint(0), 0, false);
        pointSet->ExecuteOperation(doOp);
        delete doOp;
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;

    case QMessageBox::No: 
    default:
      break;
  }
  emit PointListChanged();
}


void QmitkPointListWidget::OnLoadPointSetButtonClicked()
{
  if (!m_PointSetNode) return;

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

    bool interactionOn( m_Interactor.IsNotNull() );
    if (interactionOn)
    {
      OnEditPointSetButtonToggled(false);
    }

    m_PointSetNode->SetData( pointSet );
    m_ListView->SetPointSet( pointSet );
    
    if (interactionOn)
    {
      OnEditPointSetButtonToggled(true);
    }
  }
  catch(...)
  {
    QMessageBox::warning( this, "Load point set", QString("File reader collapsed while reading %1").arg(filename) );
  }
  emit PointListChanged();
}


void QmitkPointListWidget::OnSavePointSetButtonClicked()
{
  if (!m_PointSetNode) 
    return;
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>( m_PointSetNode->GetData() );
  if (!pointSet) 
    return; // don't write empty point sets. If application logic requires something else then do something else.
  if (pointSet->GetSize() == 0) 
    return; 
  
  // let the user choose a file
  std::string name("");

  QString fileNameProposal = QString("/%1.mps").arg(m_PointSetNode->GetName().c_str()); //"PointSet.mps";
  QString aFilename = QFileDialog::getSaveFileName( NULL, "Save point set", QDir::currentPath() + fileNameProposal, "MITK Pointset (*.mps)" );
  if ( aFilename.isEmpty() ) 
    return;

  try
  {
    // instantiate the writer and add the point-sets to write
    mitk::PointSetWriter::Pointer writer = mitk::PointSetWriter::New();
    writer->SetInput( pointSet );
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


void QmitkPointListWidget::EnableEditButton( bool enabled )
{
  m_EditAllowed = enabled;
  if (enabled == false)    
    m_BtnEdit->setEnabled(false);
  else
    m_BtnEdit->setEnabled(true);
  OnEditPointSetButtonToggled(enabled);
}
