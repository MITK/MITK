/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-20 18:43:21 +0200 (Wed, 20 May 2009) $
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <qvalidator.h>
#include <mitkImageCast.h>
#include <stdio.h>
#include <stdlib.h>
#include <mitkLevelWindowProperty.h>
#include <mitkRenderingManager.h>
#include "itkRegularStepGradientDescentOptimizer.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "QmitkBSplineRegistrationView.h"
#include "ui_QmitkBSplineRegistrationViewControls.h"
#include "mitkBSplineRegistration.h"
#include "itkImageFileReader.h"


typedef itk::Vector< float, 3 >       VectorType;
typedef itk::Image< VectorType, 3 >   DeformationFieldType;

typedef itk::ImageFileReader< DeformationFieldType > ImageReaderType;

QmitkBSplineRegistrationView::QmitkBSplineRegistrationView(QWidget* parent, Qt::WindowFlags f ) : QWidget( parent, f ),
m_FixedNode(NULL), m_MovingNode(NULL)
{
  m_Controls.setupUi(parent);

  QObject::connect( (QObject*)(m_Controls.m_PrintDeformField),
              SIGNAL(clicked()), 
              (QObject*) this,
              SLOT(PrintDeformationField()) );


  

  QObject::connect( (QObject*)(m_Controls.m_BrowseDeformationField),
              SIGNAL(clicked()), 
              (QObject*) this,
              SLOT(SelectDeformationField()) );

  connect( m_Controls.m_OptimizerSelector, SIGNAL(activated(int)), m_Controls.m_OptimizerWidgetStack, SLOT(setCurrentIndex(int)));
  connect( m_Controls.m_OptimizerSelector, SIGNAL(activated(int)), this, SLOT(OptimizerSelected(int)));
  
}

QmitkBSplineRegistrationView::~QmitkBSplineRegistrationView()
{
}


void QmitkBSplineRegistrationView::OptimizerSelected(int optimizer)
{
  HideAllOptimizerFrames();
  if(optimizer == 0)
  {
    m_Controls.m_LBFGSFrame->show();
  }
  else if(optimizer == 1)
  {
    m_Controls.m_GradientDescentFrame->show();
  }
}

void QmitkBSplineRegistrationView::HideAllOptimizerFrames()
{
  m_Controls.m_LBFGSFrame->hide();
  m_Controls.m_GradientDescentFrame->hide();
}

void QmitkBSplineRegistrationView::SelectDeformationField()
{ 
  // SELECT FOLDER DIALOG
  QFileDialog* w = new QFileDialog( this, "Select Deformation Field" );
  w->setFileMode( QFileDialog::ExistingFiles );
  w->setFilter( "Images (*.mhd)" );
  w->setDirectory("G:\\home\\vanbrugg\\testimages\\deformable"); 

  // RETRIEVE SELECTION
  if ( w->exec() != QDialog::Accepted )
  {
    return;
    cout << "Failed to load" << endl;
  }

  QStringList filenames = w->selectedFiles();
  QStringList::Iterator it = filenames.begin();
  if( it != filenames.end() ) {
    std::string filename = ( *it ).toStdString();
    ++it;     
    QString qStr = QString( filename.c_str() );
    m_Controls.m_DeformationField->setText(qStr); 
  }
   
}



void QmitkBSplineRegistrationView::PrintDeformationField()
{               
 
  ImageReaderType::Pointer reader  = ImageReaderType::New();
  reader->SetFileName(  m_Controls.m_DeformationField->text().toStdString()  );
  reader->Update();
      
  DeformationFieldType::Pointer deformationField = reader->GetOutput();
     

  typedef itk::ImageRegionIterator<DeformationFieldType> IteratorType;
  IteratorType deformIter(deformationField, deformationField->GetRequestedRegion());

  for(deformIter.GoToBegin(); !deformIter.IsAtEnd(); ++deformIter)
  {
    std::cout << deformIter.Get() << std::endl;
  }
}


void QmitkBSplineRegistrationView::CalculateTransformation()
{
  if (m_FixedNode != NULL && m_MovingNode != NULL)
  {
    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    

    mitk::BSplineRegistration::Pointer registration = mitk::BSplineRegistration::New();
    
    registration->SetSaveResult(false);
    registration->SetReferenceImage(fimage);
    registration->SetInput(mimage);
    
    // Read out optimizer parameters from the interface
    setOptimizerParameters();    

    registration->SetNumberOfGridPoints( m_Controls.m_NumberOfGridNodes->text().toInt() );
    registration->SetOptimizerParameters(m_OptimizerParameters);
    registration->SetUpdateInputImage(true);

    if(m_Controls.m_SaveDeformFieldCheck->isChecked())
    {
      // Set some parameters to save the deformation field
      registration->SetSaveDeformationField(true);
      registration->SetDeformationFileName( m_Controls.m_DeformationField->text().toStdString() );
    }


    try
    {
      registration->Update();
    }
    catch (itk::ExceptionObject& excpt)
    {
      QMessageBox::information( this, "Registration exception", excpt.GetDescription(), QMessageBox::Ok );
    }

    mitk::Image::Pointer image = registration->GetOutput();
   
    if (image.IsNotNull())
    {
      m_MovingNode->SetData(image);
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelWindow;
      levelWindow.SetAuto( image );
      levWinProp->SetLevelWindow(levelWindow);
      m_MovingNode->GetPropertyList()->SetProperty("levelwindow",levWinProp);
    }

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  } 
}

void QmitkBSplineRegistrationView::setOptimizerParameters()
{
  m_OptimizerParameters = mitk::OptimizerParameters::New();

  if(m_Controls.m_OptimizerSelector->currentText() == "LBFGSOptimizer")
  {
    m_OptimizerParameters->SetOptimizer(mitk::OptimizerParameters::LBFGSOPTIMIZER);
    m_OptimizerParameters->SetGradientConvergenceToleranceLBFGS( m_Controls.m_GradConvTolerance->text().toFloat() );
    m_OptimizerParameters->SetLineSearchAccuracyLBFGS( m_Controls.m_LineSearchAccuracy->text().toFloat() );
    m_OptimizerParameters->SetDefaultStepLengthLBFGS( m_Controls.m_DefaultStepLength->text().toFloat() );
    m_OptimizerParameters->SetNumberOfIterationsLBFGS( m_Controls.m_FunctionEvaluations->text().toInt() );
  }
  else if(m_Controls.m_OptimizerSelector->currentText() == "Gradient Descent")
  {
    m_OptimizerParameters->SetOptimizer(mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER);
    m_OptimizerParameters->SetLearningRateGradientDescent( m_Controls.m_LearningRateGradientDescent->text().toFloat() );
    m_OptimizerParameters->SetNumberOfIterationsGradientDescent (m_Controls.m_NumberOfIterationsGradientDescent->text().toInt() );
  }
}


void QmitkBSplineRegistrationView::SetFixedNode( mitk::DataNode * fixedNode )
{
  m_FixedNode = fixedNode;
}

void QmitkBSplineRegistrationView::SetMovingNode( mitk::DataNode * movingNode )
{
  m_MovingNode = movingNode;
}

