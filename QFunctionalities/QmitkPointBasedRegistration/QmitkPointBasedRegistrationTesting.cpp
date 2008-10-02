/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 11450 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkPointBasedRegistration.h" // includes mitkConfig, where BUILD_TESTING is defined
#include "QmitkPointBasedRegistrationControls.h"

#ifdef BUILD_TESTING // only if we build a test driver

#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkUserInputSimulation.h"
#include "QmitkMessageBoxHelper.h"
#include "QmitkTreeNodeSelector.h"
#include <QmitkStdMultiWidget.h>
#include "QmitkCommonFunctionality.h"
#include "qradiobutton.h"
#include "qpushbutton.h"
#include "qslider.h"
#include "qcheckbox.h"
#include "qlistbox.h"
#include <qcombobox.h>
#include <qapplication.h>
#include "QmitkMessageBoxHelper.h"
#include "QmitkPointListWidget.h"

#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>

#include <ctime>


int QmitkPointBasedRegistration::TestYourself()
{
  m_MessageBox = false;

  std::cout << std::endl;
  time_t randomInit = std::time(0);
  //randomInit = 1200058324;
  std::cout << "Initializing random number generator with " << randomInit << std::endl;
  std::srand(randomInit);

  if (m_Controls->m_FixedSelector->GetSelectedNode() == NULL)
  {
    std::cout << "No Fixed Image available! Won't apply test (l. " << __LINE__ << ")" << std::endl;
    return EXIT_SUCCESS;
  }
  mitk::DataTreeNode::Pointer node = NULL;
  if (m_Controls->m_MovingSelector->GetSelectedNode() == NULL)
  {
    node = mitk::DataTreeNode::New();
    //Create Image out of nowhere
	  mitk::Image::Pointer image;
    m_Controls->m_FixedSelector->GetSelectedNode()->GetData();
	  mitk::PixelType pt(typeid(int));
    unsigned int dim[]={100,100,20};

    std::cout << "Creating image: ";
	  image = mitk::Image::New();
	  image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
    int *p = (int*)image->GetData();

    int size = dim[0]*dim[1]*dim[2];
    int i;
    for(i=0; i<size; ++i, ++p)
    {
      *p=i;
    }
    node->SetData(image);
    node->SetProperty("name", mitk::StringProperty::New("new created node for PointBasedRegistrationTest"));
    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelWindow;
    levelWindow.SetAuto( image );
    levWinProp->SetLevelWindow(levelWindow);
    node->GetPropertyList()->SetProperty("levelwindow",levWinProp);
    mitk::DataTreeIteratorClone it;
    it = *m_Controls->m_FixedSelector->GetSelectedIterator();
    it->GoToParent();
    it->Add(node);
    std::cout<<"[PASSED]"<<std::endl;
  }

  if (m_Controls->m_MovingSelector->GetSelectedNode() == NULL)
  {
    std::cerr << "No Moving Image available! Won't apply test (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }

  // test Show Images Red/Green
  std::cout << "Test show images red/green:" << std::endl;
  QmitkUserInputSimulation::MouseClick( m_Controls->m_ShowRedGreenValues, Qt::LeftButton );
  m_Controls->m_ShowRedGreenValues->setChecked(true);
  std::cout << " [PASSED]" << std::endl;

  // test for Select model
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedPointListWidget->m_SetPoints, Qt::LeftButton );
  if (!m_Controls->m_FixedPointListWidget->m_SetPoints->isOn())
  {
    std::cerr << "Select Fixed Image was not toggled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  if (m_Controls->m_MovingPointListWidget->m_SetPoints->isOn())
  {
    std::cerr << "Select Moving Image was toggled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_SetPoints, Qt::LeftButton );
  if (!m_Controls->m_MovingPointListWidget->m_SetPoints->isOn())
  {
    std::cerr << "Select Moving Image was not toggled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  if (m_Controls->m_FixedPointListWidget->m_SetPoints->isOn())
  {
    std::cerr << "Select Fixed Image was toggled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_SetPoints, Qt::LeftButton );
  if (m_Controls->m_MovingPointListWidget->m_SetPoints->isOn())
  {
    std::cerr << "Select Moving Image was toggled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  if (m_Controls->m_FixedPointListWidget->m_SetPoints->isOn())
  {
    std::cerr << "Select Fixed Image was toggled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }

  // test for undobutton at start
  if (m_Controls->m_UndoTransformation->isEnabled())
  {
    std::cerr << "Undo Transformation was enabled but there was no transformation before (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }

  // test for redobutton at start
  if (m_Controls->m_RedoTransformation->isEnabled())
  {
    std::cerr << "Redo Transformation was enabled but there was no transformation before (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }

  // test reinit buttons
  std::cout << "Test reinit buttons:" << std::endl;
  QmitkUserInputSimulation::MouseClick( m_Controls->m_ReinitFixedButton, Qt::LeftButton );
  QmitkUserInputSimulation::MouseClick( m_Controls->m_ReinitMovingButton, Qt::LeftButton );
  m_Controls->globalReinitClicked();
  std::cout << " [PASSED]" << std::endl;

  // test opacity slider
  std::cout << "Test opacity slider:" << std::endl;
  m_Controls->m_OpacitySlider->setValue(m_Controls->m_OpacitySlider->minValue());
  m_Controls->m_OpacitySlider->setValue(m_Controls->m_OpacitySlider->maxValue());
  std::cout << " [PASSED]" << std::endl;

  bool testOK = this->TestAllTools();

  // clean up
  if (node.IsNotNull())
  {
    mitk::DataStorage::GetInstance()->Remove(node);
    node = NULL;
  }
  
  // recenter all remaining datatreenodes
  m_Controls->globalReinitClicked();
  
  if (testOK)
  {
    std::cout << "Whole functionality testing [PASSED]" << std::endl;
    return EXIT_SUCCESS;
  }
  else
  {
    std::cout << "Whole functionality testing [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
}

bool QmitkPointBasedRegistration::TestAllTools()
{
  QWidget* sliceWidget;
  m_MultiWidget->changeLayoutToDefault();
  
  std::cout << "Creating landmarks:" << std::endl;
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedPointListWidget->m_SetPoints, Qt::LeftButton );

  for (unsigned int window = 1; window < 4; ++window)
  {
    switch (window)
    {
      case 1:
        sliceWidget = m_MultiWidget->mitkWidget1; // IMPORTANT to send the events to the renderwindow and not to the multiwidget or one selectableglwidget
        break;
      case 2:
        sliceWidget = m_MultiWidget->mitkWidget2;
        break;
      case 3:
      default:
        sliceWidget = m_MultiWidget->mitkWidget3;
        break;
    }
    float w = (float)sliceWidget->width()-10;
    float h = (float)sliceWidget->height()-10;
    for (unsigned int i = 0; i < 5; ++i )
    {
      double r;
      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float x = r * w;

      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float y = r * h;
      
      QmitkUserInputSimulation::MouseDown( sliceWidget, (int)x+5, (int)y+5, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, (int)x+5, (int)y+5, Qt::LeftButton, Qt::ShiftButton );
    }
  }

  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_SetPoints, Qt::LeftButton );

  for (unsigned int window = 1; window < 4; ++window)
  {
    switch (window)
    {
      case 1:
        sliceWidget = m_MultiWidget->mitkWidget1; // IMPORTANT to send the events to the renderwindow and not to the multiwidget or one selectableglwidget
        break;
      case 2:
        sliceWidget = m_MultiWidget->mitkWidget2;
        break;
      case 3:
      default:
        sliceWidget = m_MultiWidget->mitkWidget3;
        break;
    }
    float w = (float)sliceWidget->width();
    float h = (float)sliceWidget->height();
    for (unsigned int i = 0; i < 5; ++i )
    {
      double r;
      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float x = r * w;

      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float y = r * h;
      QmitkUserInputSimulation::MouseDown( sliceWidget, (int)x, (int)y, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, (int)x, (int)y, Qt::LeftButton, Qt::ShiftButton );
    }
  }
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_SetPoints, Qt::LeftButton );
  std::cout << " [PASSED]" << std::endl;

/////// test for deleting points from fixed point list
  std::cout << "Delete Landmarks out of order:" << std::endl;
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(4, true);
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_FixedPointListWidget->InteractivePointList, Qt::Key_Return );
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(8,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(13,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(0,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(7,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(7,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(3,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(4,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  for (int i = 0; i < 7; i++)
  {
    m_Controls->m_FixedPointListWidget->InteractivePointList->setSelected(0,true);
    QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  }
  /////// test for deleting points from moving point list
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(0,true);
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_MovingPointListWidget->InteractivePointList, Qt::Key_Return );
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(14,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(11,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(8,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(6,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(2,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(2,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(2,true);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  for (int i = 0; i < 7; i++)
  {
    m_Controls->m_MovingPointListWidget->InteractivePointList->setSelected(0,true);
    QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  }
  std::cout << " [PASSED]" << std::endl;

////// create new points
  std::cout << "Create new landmarks:" << std::endl;
  std::cout << "Creating landmarks:" << std::endl;
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedPointListWidget->m_SetPoints, Qt::LeftButton );

  for (unsigned int window = 1; window < 4; ++window)
  {
    switch (window)
    {
      case 1:
        sliceWidget = m_MultiWidget->mitkWidget1; // IMPORTANT to send the events to the renderwindow and not to the multiwidget or one selectableglwidget
        break;
      case 2:
        sliceWidget = m_MultiWidget->mitkWidget2;
        break;
      case 3:
      default:
        sliceWidget = m_MultiWidget->mitkWidget3;
        break;
    }
    float w = (float)sliceWidget->width()-10;
    float h = (float)sliceWidget->height()-10;
    for (unsigned int i = 0; i < 5; ++i )
    {
      double r;
      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float x = r * w;

      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float y = r * h;
      
      QmitkUserInputSimulation::MouseDown( sliceWidget, (int)x+5, (int)y+5, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, (int)x+5, (int)y+5, Qt::LeftButton, Qt::ShiftButton );
    }
  }

  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_SetPoints, Qt::LeftButton );

  for (unsigned int window = 1; window < 4; ++window)
  {
    switch (window)
    {
      case 1:
        sliceWidget = m_MultiWidget->mitkWidget1; // IMPORTANT to send the events to the renderwindow and not to the multiwidget or one selectableglwidget
        break;
      case 2:
        sliceWidget = m_MultiWidget->mitkWidget2;
        break;
      case 3:
      default:
        sliceWidget = m_MultiWidget->mitkWidget3;
        break;
    }
    float w = (float)sliceWidget->width();
    float h = (float)sliceWidget->height();
    for (unsigned int i = 0; i < 5; ++i )
    {
      double r;
      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float x = r * w;

      r = ( (double)rand() / ((double)(RAND_MAX)+(double)(1)) );
      float y = r * h;
      QmitkUserInputSimulation::MouseDown( sliceWidget, (int)x, (int)y, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, (int)x, (int)y, Qt::LeftButton, Qt::ShiftButton );
    }
  }
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_SetPoints, Qt::LeftButton );
  std::cout << " [PASSED]" << std::endl;

//// Rigid with ICP
  // a helper object to close popping up message boxes
  QmitkMessageBoxHelper* helper1 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper1, SIGNAL(DialogFound(QWidget*)), this, SLOT(RegistrationErrorDialogFound(QWidget*)) );
  helper1->WaitForDialogAndCallback( "QMessageBox" );
  if (m_Controls->m_Calculate->isEnabled())
  {
    QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
    std::cout << "Rigid with ICP [PASSED]" << std::endl;
  }
  else
  {
    std::cout << "Rigid with ICP [FAILED]" << std::endl;
    helper1->StopWaitForDialogAndCallback();
    return false;
  }
  helper1->StopWaitForDialogAndCallback();
  if (!m_MessageBox)
  {
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      std::cout << "Undo was enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_RedoTransformation, Qt::LeftButton );
      std::cout << "Redo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Redo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      std::cout << "Redo was enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      std::cout << "Undo was enabled [FAILED]" << std::endl;
      return false;
    }
  }
  m_MessageBox = false;


//// Similarity with ICP
  QmitkMessageBoxHelper* helper2 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper2, SIGNAL(DialogFound(QWidget*)), this, SLOT(RegistrationErrorDialogFound(QWidget*)) );
  helper2->WaitForDialogAndCallback( "QMessageBox" );
  m_Controls->m_SelectedTransformationClass->setFocus();
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_SelectedTransformationClass, Qt::Key_Down );
  if (m_Controls->m_Calculate->isEnabled())
  {
    QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
    std::cout << "Similarity with ICP [PASSED]" << std::endl;
  }
  else
  {
    std::cout << "Similarity with ICP [FAILED]" << std::endl;
    helper2->StopWaitForDialogAndCallback();
    return false;
  }
  helper2->StopWaitForDialogAndCallback();
  if(!m_MessageBox)
  {
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_RedoTransformation, Qt::LeftButton );
      std::cout << "Redo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Redo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
  }
  m_MessageBox = false;


//// Affine with ICP
  QmitkMessageBoxHelper* helper3 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper3, SIGNAL(DialogFound(QWidget*)), this, SLOT(RegistrationErrorDialogFound(QWidget*)) );
  helper3->WaitForDialogAndCallback( "QMessageBox" );
  m_Controls->m_SelectedTransformationClass->setFocus();
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_SelectedTransformationClass, Qt::Key_Down );
  if (m_Controls->m_Calculate->isEnabled())
  {
    QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
    std::cout << "Affine with ICP [PASSED]" << std::endl;
  }
  else
  {
    std::cout << "Affine with ICP [FAILED]" << std::endl;
    helper3->StopWaitForDialogAndCallback();
    return false;
  }
  helper3->StopWaitForDialogAndCallback();
  if(!m_MessageBox)
  {
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_RedoTransformation, Qt::LeftButton );
      std::cout << "Redo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Redo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
  }
  m_MessageBox = false;

//// Rigid without ICP
  m_Controls->m_SelectedTransformationClass->setFocus();  
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_SelectedTransformationClass, Qt::Key_Down );
  // a helper object to close popping up message boxes
  QmitkMessageBoxHelper* helper4 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper4, SIGNAL(DialogFound(QWidget*)), this, SLOT(RegistrationErrorDialogFound(QWidget*)) );
  helper4->WaitForDialogAndCallback( "QMessageBox" );
  if (m_Controls->m_Calculate->isEnabled())
  {
    QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
    std::cout << "Rigid [PASSED]" << std::endl;
  }
  else
  {
    std::cout << "Rigid [FAILED]" << std::endl;
    helper4->StopWaitForDialogAndCallback();
    return false;
  }
  helper4->StopWaitForDialogAndCallback();
  if(!m_MessageBox)
  {
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_RedoTransformation, Qt::LeftButton );
      std::cout << "Redo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Redo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
  }
  m_MessageBox = false;

//// Similarity without ICP
  m_Controls->m_SelectedTransformationClass->setFocus();
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_SelectedTransformationClass, Qt::Key_Down );
  // a helper object to close popping up message boxes
  QmitkMessageBoxHelper* helper5 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper5, SIGNAL(DialogFound(QWidget*)), this, SLOT(RegistrationErrorDialogFound(QWidget*)) );
  helper5->WaitForDialogAndCallback( "QMessageBox" );
  if (m_Controls->m_Calculate->isEnabled())
  {
    QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
    std::cout << "Similarity [PASSED]" << std::endl;
  }
  else
  {
    std::cout << "Similarity [FAILED]" << std::endl;
    helper5->StopWaitForDialogAndCallback();
    return false;
  }
  helper5->StopWaitForDialogAndCallback();
  if(!m_MessageBox)
  {
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_RedoTransformation, Qt::LeftButton );
      std::cout << "Redo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Redo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
  }
  m_MessageBox = false;

//// Affine without ICP
  m_Controls->m_SelectedTransformationClass->setFocus();
  QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_SelectedTransformationClass, Qt::Key_Down );
  // a helper object to close popping up message boxes
  QmitkMessageBoxHelper* helper6 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper6, SIGNAL(DialogFound(QWidget*)), this, SLOT(RegistrationErrorDialogFound(QWidget*)) );
  helper6->WaitForDialogAndCallback( "QMessageBox" );
  if (m_Controls->m_Calculate->isEnabled())
  {
    QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
    std::cout << "Affine [PASSED]" << std::endl;
  }
  else
  {
    std::cout << "Affine [FAILED]" << std::endl;
    helper6->StopWaitForDialogAndCallback();
    return false;
  }
  helper6->StopWaitForDialogAndCallback();
  if(!m_MessageBox)
  {
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_RedoTransformation, Qt::LeftButton );
      std::cout << "Redo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Redo was not enabled [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_UndoTransformation, Qt::LeftButton );
      std::cout << "Undo [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "Undo was not enabled [FAILED]" << std::endl;
      return false;
    }
  }
  m_MessageBox = false;
/*
//// LandmarkWarping
  if (dynamic_cast<mitk::Image*>(m_Controls->m_FixedSelector->GetSelectedNode()->GetData()) != NULL &&
      (dynamic_cast<mitk::Image*>(m_Controls->m_FixedSelector->GetSelectedNode()->GetData())->GetDimension() == 2 || 
      dynamic_cast<mitk::Image*>(m_Controls->m_FixedSelector->GetSelectedNode()->GetData())->GetDimension() == 3))
  {
    m_Controls->m_SelectedTransformationClass->setFocus();
    QmitkUserInputSimulation::KeyboardTypeKey( m_Controls->m_SelectedTransformationClass, Qt::Key_Down );
    if (m_Controls->m_Calculate->isEnabled())
    {
      QmitkUserInputSimulation::MouseClick( m_Controls->m_Calculate, Qt::LeftButton );
      std::cout << "LandmarkWarping [PASSED]" << std::endl;
    }
    else
    {
      std::cout << "LandmarkWarping [FAILED]" << std::endl;
      return false;
    }
    if (m_Controls->m_UndoTransformation->isEnabled())
    {
      std::cout << "Undo [FAILED]" << std::endl;
      return false;
    }
    else
    {
      std::cout << "Undo [PASSED]" << std::endl;
    }
    if (m_Controls->m_RedoTransformation->isEnabled())
    {
      std::cout << "Redo [FAILED]" << std::endl;
      return false;
    }
    else
    {
      std::cout << "Redo [PASSED]" << std::endl;
    }
  }*/

//// end registrationmethods test

  QmitkMessageBoxHelper* helper7 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper7, SIGNAL(DialogFound(QWidget*)), this, SLOT(ClearPointSetDialogFound(QWidget*)) );
  helper7->WaitForDialogAndCallback( "QMessageBox" );
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedPointListWidget->m_ClearPointSet, Qt::LeftButton );

  QmitkMessageBoxHelper* helper8 = new QmitkMessageBoxHelper(m_Controls);
  connect( helper8, SIGNAL(DialogFound(QWidget*)), this, SLOT(ClearPointSetDialogFound(QWidget*)) );
  helper8->WaitForDialogAndCallback( "QMessageBox" );
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingPointListWidget->m_ClearPointSet, Qt::LeftButton );

  if(this->m_FixedLandmarks->GetSize() > 0)
  {
    std::cout << "Not all fixed points are deleted! [FAILED]" << std::endl;
    return false;
  }
  if(m_MovingLandmarks->GetSize() > 0)
  {
    std::cout << "Not all moving points are deleted! [FAILED]" << std::endl;
    return false;
  }

  return true;
}

void QmitkPointBasedRegistration::RegistrationErrorDialogFound( QWidget* widget )
{
  if (!widget) return;

  // close message box
  widget->close();
  std::cout<<"Message box closed!"<<std::endl;
  m_MessageBox = true;
}

void QmitkPointBasedRegistration::ClearPointSetDialogFound( QWidget* widget )
{
  if (!widget) return;

  // close message box
  QMessageBox msgBox = (QMessageBox*)(widget);
  QmitkUserInputSimulation::KeyboardTypeKey( widget, Qt::Key_Return );
  std::cout<<"Message box closed!"<<std::endl;
  m_MessageBox = true;
}


#endif
