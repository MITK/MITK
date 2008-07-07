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
#include "qtable.h"
#include <qcombobox.h>
#include <qapplication.h>
#include "QmitkMessageBoxHelper.h"

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
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedSelected, Qt::LeftButton );
  m_Controls->m_FixedSelected->animateClick();
  if (!m_Controls->m_FixedSelected->isEnabled())
  {
    std::cerr << "Select Fixed Image was not enabled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingSelected, Qt::LeftButton );
  m_Controls->m_MovingSelected->animateClick();
  if (!m_Controls->m_MovingSelected->isEnabled())
  {
    std::cerr << "Select Moving Image was not enabled (l. " << __LINE__ << ")" << std::endl;
    return EXIT_FAILURE;
  }
  
  QmitkUserInputSimulation::MouseClick( m_Controls->m_NoModelSelected, Qt::LeftButton );
  m_Controls->m_NoModelSelected->animateClick();
  if (!m_Controls->m_NoModelSelected->isEnabled())
  {
    std::cerr << "Select Both was not enabled (l. " << __LINE__ << ")" << std::endl;
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
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedReinit, Qt::LeftButton );
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingReinit, Qt::LeftButton );
  QmitkUserInputSimulation::MouseClick( m_Controls->m_GlobalReinit, Qt::LeftButton );
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
    mitk::DataTreeIteratorClone it = mitk::DataTree::GetIteratorToNode( GetDataTreeIterator()->GetTree(), node );
    m_MovingNode = NULL;
    it->Remove();
    node = NULL;
    mitk::DataTreeIteratorClone it2 = mitk::DataTree::GetIteratorToNode( GetDataTreeIterator()->GetTree(), m_FixedPointSetNode );
    it2->Remove();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  // recenter all remaining datatreenodes
  QmitkUserInputSimulation::MouseClick( m_Controls->m_GlobalReinit, Qt::LeftButton );
  
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
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedSelected, Qt::LeftButton );
  m_Controls->m_FixedSelected->animateClick();
  addFixedInteractor();
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedReinit, Qt::LeftButton );

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
      
      QmitkUserInputSimulation::MouseDown( sliceWidget, x+5, y+5, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, x+5, y+5, Qt::LeftButton, Qt::ShiftButton );
    }
  }

  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingSelected, Qt::LeftButton );
  m_Controls->m_MovingSelected->animateClick();
  addMovingInteractor();
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingReinit, Qt::LeftButton );

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
      QmitkUserInputSimulation::MouseDown( sliceWidget, x, y, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, x, y, Qt::LeftButton, Qt::ShiftButton );
    }
  }
  QmitkUserInputSimulation::MouseClick( m_Controls->m_NoModelSelected, Qt::LeftButton );  
  m_Controls->m_NoModelSelected->animateClick();
  bothSelected();
  QmitkUserInputSimulation::MouseClick( m_Controls->m_GlobalReinit, Qt::LeftButton );
  std::cout << " [PASSED]" << std::endl;

/////// test for deleting points from table
  std::cout << "Delete Landmarks out of order:" << std::endl;
  m_Controls->m_PointsetTable->setCurrentCell(4,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(8,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(13,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(0,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(7,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(7,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(3,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(4,0);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  for (int i = 0; i < 7; i++)
  {
    m_Controls->m_PointsetTable->setCurrentCell(0,0);
    QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  }
  m_Controls->m_PointsetTable->setCurrentCell(0,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(14,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(11,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(8,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(6,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(2,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(2,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  m_Controls->m_PointsetTable->setCurrentCell(2,1);
  QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  for (int i = 0; i < 7; i++)
  {
    m_Controls->m_PointsetTable->setCurrentCell(0,1);
    QmitkUserInputSimulation::KeyboardTypeKey( sliceWidget, Qt::Key_Delete );
  }
  std::cout << " [PASSED]" << std::endl;

////// create new points
  std::cout << "Create new landmarks:" << std::endl;
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedSelected, Qt::LeftButton );
  m_Controls->m_FixedSelected->animateClick();
  addFixedInteractor();
  QmitkUserInputSimulation::MouseClick( m_Controls->m_FixedReinit, Qt::LeftButton );

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
      
      QmitkUserInputSimulation::MouseDown( sliceWidget, x+5, y+5, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, x+5, y+5, Qt::LeftButton, Qt::ShiftButton );
    }
  }

  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingSelected, Qt::LeftButton );
  m_Controls->m_MovingSelected->animateClick();
  addMovingInteractor();
  QmitkUserInputSimulation::MouseClick( m_Controls->m_MovingReinit, Qt::LeftButton );

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
      QmitkUserInputSimulation::MouseDown( sliceWidget, x, y, Qt::LeftButton, Qt::ShiftButton );
      QmitkUserInputSimulation::MouseRelease( sliceWidget, x, y, Qt::LeftButton, Qt::ShiftButton );
    }
  }
  QmitkUserInputSimulation::MouseClick( m_Controls->m_NoModelSelected, Qt::LeftButton );  
  m_Controls->m_NoModelSelected->animateClick();
  bothSelected();
  QmitkUserInputSimulation::MouseClick( m_Controls->m_GlobalReinit, Qt::LeftButton );
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

  QmitkUserInputSimulation::MouseClick( m_Controls->m_ResetPointsets, Qt::LeftButton );

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

#endif
