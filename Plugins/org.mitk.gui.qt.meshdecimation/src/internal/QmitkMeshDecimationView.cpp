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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMeshDecimationView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <QTimer>
#include <QApplication>

// mitk
#include <mitkSurface.h>
#include <mitkNodePredicateDataType.h>
#include <mitkRenderingManager.h>

// VTK
#include <vtkSmartPointer.h>
#include <vtkQuadricDecimation.h>
#include <vtkDecimatePro.h>

const std::string QmitkMeshDecimationView::VIEW_ID = "org.mitk.views.meshdecimation";

QmitkMeshDecimationView::QmitkMeshDecimationView()
: QmitkAbstractView()
, m_Controls( 0 )
, m_MessageBox( 0 )
, m_MessageBoxDisplayTime( 5 )
, m_MessageBoxTimer( new QTimer(this) )
{
  m_MessageBoxTimer->setInterval(1000);
  connect(m_MessageBoxTimer, SIGNAL(timeout()), this, SLOT(MessageBoxTimer()));
}

QmitkMeshDecimationView::~QmitkMeshDecimationView()
{
}

void QmitkMeshDecimationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkMeshDecimationView;
    m_Controls->setupUi( parent );

    connect((QObject*)(m_Controls->Decimate), SIGNAL(clicked()), this, SLOT(Decimate()));
  }
}

void QmitkMeshDecimationView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes)
{
  this->m_Node = 0;
  this->DisableGui();

  for( int i=0; i<nodes.size(); ++i )
  {
    mitk::BaseData* baseData = nodes.at(i)->GetData();
    if( dynamic_cast<mitk::Surface*>( baseData ) != 0 )
    {
      this->m_Node = nodes.at(i);
      this->m_Controls->NodeLabel->setEnabled( true );
      this->m_Controls->TargetReduction->setEnabled( true );
      this->m_Controls->Decimate->setEnabled( true );
      this->m_Controls->DecimationType->setEnabled( true );
      break;
    }
  }
  this->UpdateNodeLabel();
}

void QmitkMeshDecimationView::SetFocus()
{
  m_Controls->NodeLabel->setFocus();
}

void QmitkMeshDecimationView::DisableGui()
{
  this->m_Controls->NodeLabel->setEnabled( false );
  this->m_Controls->TargetReduction->setEnabled( false );
  this->m_Controls->Decimate->setEnabled( false );
  this->m_Controls->DecimationType->setEnabled( false );
}

void QmitkMeshDecimationView::UpdateNodeLabel()
{
  QString nodeLabel = "none selected";
  if (this->m_Node.IsNotNull())
  {
      vtkPolyData* polyData = static_cast<mitk::Surface*>(this->m_Node->GetData())->GetVtkPolyData();
      nodeLabel = QString::fromStdString( this->m_Node->GetName() );
      nodeLabel.append(" (");
      nodeLabel.append( QString("%1").arg( polyData->GetNumberOfCells() ) );
      nodeLabel.append(" cells)");
  }
  this->m_Controls->NodeLabel->setText( nodeLabel );
}

void QmitkMeshDecimationView::Decimate()
{
  if (this->m_Node.IsNotNull())
  {
    this->WaitCursorOn();
    QString error;
    try
    {
      vtkPolyData* polyData = static_cast<mitk::Surface*>(this->m_Node->GetData())->GetVtkPolyData();
      int startingNumberOfCells = polyData->GetNumberOfCells();
      if( startingNumberOfCells == 0 )
        throw std::logic_error("Could not reduce Mesh since it is already empty.");


      double targetReduction = static_cast<double>(this->m_Controls->TargetReduction->value()) / 100.0;

      vtkPolyData* newPolyData = 0;
      if( this->m_Controls->DecimationType->currentText() == "DecimatePro" )
      {
        vtkSmartPointer<vtkDecimatePro> decimator = vtkDecimatePro::New();
        decimator->SetInput( polyData );
        decimator->SetTargetReduction( targetReduction );
        decimator->SetPreserveTopology( 1 );
        decimator->Update();
        newPolyData = decimator->GetOutput();
      }
      else
      {
        vtkSmartPointer<vtkQuadricDecimation> decimator = vtkQuadricDecimation::New();
        decimator->SetInput( polyData );
        decimator->SetTargetReduction( targetReduction );
        decimator->Update();
        newPolyData = decimator->GetOutput();
      }

      if( newPolyData != 0 )
      {
        static_cast<mitk::Surface*>(this->m_Node->GetData())->SetVtkPolyData(newPolyData);
        this->m_Node->Update();
        this->UpdateNodeLabel();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }

      int endingNumberOfCells = newPolyData->GetNumberOfCells();
      int reducedCells = startingNumberOfCells - endingNumberOfCells;

      QString message;
      if( reducedCells == 0 )
        message = "Decimation was not able to reduce any more cells from Mesh.";
      else
        message = QString("Decimation successful. %1 cells removed").arg(reducedCells);

      this->WaitCursorOff();
      this->ShowAutoCloseMessageDialog(message);
    }
    catch(std::exception& e)
    {
      this->WaitCursorOff();
      error = QString::fromStdString(e.what());
    }
    catch(...)
    {
      this->WaitCursorOff();
      error = "An unknown error occured. Please report error at bugs.mitk.org.";
    }
    if( !error.isEmpty() )
      this->ShowAutoCloseErrorDialog(error);
  }
  else
    this->DisableGui();
}

void QmitkMeshDecimationView::MessageBoxTimer()
{
  m_MessageBoxDisplayTime--;
  QString text = m_MessageBoxText;
  text.append( QString(" (Will close in %1 seconds)").arg(m_MessageBoxDisplayTime) );
  m_MessageBox->setText( text );

  if( m_MessageBoxDisplayTime == 0 )
  {
    m_MessageBox->accept();
  }
}

void QmitkMeshDecimationView::ShowAutoCloseErrorDialog( const QString& error )
{
  m_MessageBox = new QMessageBox(QMessageBox::Warning, "Warning", error, QMessageBox::Ok );
  m_MessageBoxDisplayTime = 5;
  m_MessageBoxText = error;

  this->MessageBoxTimer();
  m_MessageBoxTimer->start();
  m_MessageBox->exec();
  m_MessageBoxTimer->stop();

  delete m_MessageBox;
  m_MessageBox = 0;
}

void QmitkMeshDecimationView::ShowAutoCloseMessageDialog( const QString& message )
{
  m_MessageBox = new QMessageBox(QMessageBox::Information, "Information", message, QMessageBox::Ok );
  m_MessageBoxDisplayTime = 5;
  m_MessageBoxText = message;

  this->MessageBoxTimer();
  m_MessageBoxTimer->start();
  m_MessageBox->exec();
  m_MessageBoxTimer->stop();

  delete m_MessageBox;
  m_MessageBox = 0;
}
