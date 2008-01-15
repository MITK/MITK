/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkSliceBasedSegmentation.h" // includes mitkConfig, where BUILD_TESTING is defined

#ifdef BUILD_TESTING // only if we build a test driver

#include "QmitkSliceBasedSegmentationControls.h"

#include "QmitkToolReferenceDataSelectionBox.h"
#include "QmitkToolSelectionBox.h"
#include "QmitkNewSegmentationDialog.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"

#include "QmitkUserInputSimulation.h"
#include "QmitkMessageBoxHelper.h"
#include "QmitkSlicesInterpolator.h" // bad, bad - but I cannot figure out where to click for  the checkbox on the interpolation group box

#include "mitkMemoryUtilities.h"
#include "mitkOrganTypeProperty.h"

#include <qwidgetlist.h>
#include <qobjectlist.h>
#include <qtoolbutton.h>
#include <qlineedit.h>

#include <ctime>

/**
 * \brief Test entry point
 */
bool QmitkSliceBasedSegmentation::TestYourself()
{
  std::cout << std::endl;
  time_t randomInit = std::time(0);
  randomInit = 0; // some errors on muhu (fixed)
  std::cout << "Initializing random number generator with " << randomInit << std::endl;
  std::srand(randomInit);

  // a helper object to close popping up message boxes
  QmitkMessageBoxHelper messageBoxHelper(this);

  // check if there is a tool manager and if we have some data
  //  - having no tool manager is not ok
  //  - having no data is ok, then we just have no work to do
  mitk::ToolManager* toolManager = m_Controls->m_ToolReferenceDataSelectionBox->GetToolManager();
  if (!toolManager) 
  {
    std::cerr << "Couldn't get a ToolManager object (l. " << __LINE__ << ")" << std::endl;
    return false;
  }

  if ( toolManager->GetReferenceData(0) == NULL )
  {
    std::cerr << "No possible reference image in scene. Won't apply test (l. " << __LINE__ << ")" << std::endl;
    return true;
  }

  // Test: click the "New segmentation" button, expect: new segmentation is selected afterwards
  std::cout << "Creating a new segmentation: " << std::flush;

  // we expect a new window of type "QmitkNewSegmentationDialog" to pop up
  // here we ask messageBoxHelper to wait for that window in a separate thread and tell us when the window shows up
  connect( &messageBoxHelper, SIGNAL(DialogFound(QWidget*)), this, SLOT(TestOnNewSegmentationDialogFound(QWidget*)) );
  messageBoxHelper.WaitForDialogAndCallback( "QmitkNewSegmentationDialog" );
  QmitkUserInputSimulation::MouseClick(    m_Controls->btnNewSegmentation, Qt::LeftButton );

  // check if a new segmentation was created. After clicking "New" the new segmentation should be selected automatically
  if ( !toolManager->GetWorkingData(0) )
  {
    std::cerr << "Segmentation was perhaps generated after clicking on 'New', but no segmentation was selected as active (l. " << __LINE__ << ")" << std::endl;
    return false;
  }
  else
  {
    // checking some things about the new segmentation
    mitk::DataTreeNode* segmentationNode = toolManager->GetWorkingData(0);
    std::string name;
    segmentationNode->GetName( name );
    if ( name.find( "testname" ) == std::string::npos )
    {
      std::cerr << "New segmentation name is wrong (expected '...testname')." << std::endl;
      return false;
    }

    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( segmentationNode->GetData() );
    if ( image.IsNull() )
    {
      std::cerr << "New segmentation is no image!" << std::endl;
      return false;
    }

    mitk::OrganTypeProperty::Pointer organTypeProperty = dynamic_cast<mitk::OrganTypeProperty*>( image->GetProperty("organ type").GetPointer() );
    if ( organTypeProperty.IsNull() )
    {
      std::cerr << "New segmentation has no organ type associated." << std::endl;
      return false;
    }

    if ( ! (*(image->GetPixelType().GetTypeId()) == typeid(unsigned char)) )
    {
      std::cerr << "New segmentation data type is wrong (should be unsigned char)." << std::endl;
      return false;
    }

  }

  std::cout << "[PASSED]" << std::endl;
  
  // Test: tools should be enabled when a segmentation is active
  if ( !m_Controls->m_ToolSelectionBox->isEnabled() )
  {
    std::cerr << "Tools should be enabled after creating a new segmentation (l. " << __LINE__ << ")" << std::endl;
    return false;
  }
  std::cout << "[PASSED]" << std::endl;
 
#ifndef MITK_FAST_TESTING
  // test if interpolation does not crash
  // THIS FIRST, because the drawing area is still clear here
  if (!TestInterpolation()) return false;
#endif

  // Test: test all the tools
  if (!TestAllTools()) return false;
  
  std::cout << "Cropping active segmentation: " << std::flush;
  QmitkUserInputSimulation::MouseClick(    m_Controls->btnAutoCropSegmentation, Qt::LeftButton );
  
  // Test: click the "Delete segmentation" button, expect: no segmentations selected afterwards
  std::cout << "Deleting active segmentation: " << std::flush;

  // again use a helper to close a standard dialog after some time.
  messageBoxHelper.CloseMessageBoxes(1, 1000); // click first button (yes) in messagebox after 1000 msec
  QmitkUserInputSimulation::MouseClick(    m_Controls->btnDeleteSegmentation, Qt::LeftButton );

  // nothing should be selected as active segmenation anymore
  if ( toolManager->GetWorkingData(0) )
  {
    std::cerr << "Segmentation was perhaps deleted, but something is still selected as active segmentation (l. " << __LINE__ << ")" << std::endl;
    return false;
  }
  std::cout << "[PASSED]" << std::endl;
  
  // done
  std::cout << "Whole functionality testing [PASSED]" << std::endl;

  return true;
}

bool QmitkSliceBasedSegmentation::TestAllTools()
{
  QWidget* widget = m_Controls->m_ToolSelectionBox;
  QObjectList* childList = widget->queryList( "QToolButton" );
  QObjectListIt childIter( *childList ); 

  QObject* child;
  while ( (child = childIter.current()) )
  {
    QToolButton* button = static_cast<QToolButton*>(child);

    std::cout << "Using '" << button->textLabel().ascii() << "' tool..." << std::flush;
    if (!button->isOn()) // if not yet pressed, then press it now (otherwise, DONT press because that would deactivate the tool)
    {
      QmitkUserInputSimulation::MouseClick( button, Qt::LeftButton );
    }

    mitk::Tool* tool = m_Controls->m_ToolSelectionBox->GetToolManager()->GetActiveTool();
    if (!tool)
    {
      std::cerr << "Clicked a button, but no tool was activated! (l. " << __LINE__ << ")" << std::endl;
      return false;
    }
    
    QWidget* sliceWidget;
    
    for (unsigned int window = 1; window < 4; ++window)
    {
      switch (window)
      {
        case 1:
          sliceWidget = m_MultiWidget->GetRenderWindow1(); // IMPORTANT to send the events to the renderwindow and not to the multiwidget or one selectableglwidget
          break;
        case 2:
          sliceWidget = m_MultiWidget->GetRenderWindow2();
          break;
        case 3:
        default:
          sliceWidget = m_MultiWidget->GetRenderWindow3();
          break;
      }
/*
      std::cout << " circle " << std::flush;
      QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.25, 0.25, 0.2 );
      std::cout << " circle " << std::flush;
      QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.75, 0.75, 0.2 );
*/
#ifdef MITK_FAST_TESTING
      std::cout << " circle " << std::flush;
      QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.5, 0.5, 0.3 );
#else
      std::cout << " circle " << std::flush;
      QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.75, 0.25, 0.2 );
      std::cout << " circle " << std::flush;
      QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.25, 0.75, 0.2 );

      std::cout << " chaos " << std::flush;
      QmitkUserInputSimulation::MouseDrawRandom( sliceWidget, Qt::LeftButton, 50 ); // random points

      if (    std::string("Region growing") == tool->GetName() // little more stress for problematic tools
           || std::string("Correction") == tool->GetName()) 
      {
        for (unsigned int j = 0; j < 20; ++j)
        {
          QmitkUserInputSimulation::MouseDrawRandom( sliceWidget, Qt::LeftButton, 5 ); // random points
        }
      }
#endif
    }

    std::cout << "[PASSED]" << std::endl;

    ++childIter;
  }

  delete childList;

  return true;
}

bool QmitkSliceBasedSegmentation::TestInterpolation()
{
  std::cout << "Testing interpolation" << std::endl;

  QWidget* widget = m_Controls->m_ToolSelectionBox;
  QObjectList* childList = widget->queryList( "QToolButton" );
  QObjectListIt childIter( *childList ); 

  // look for the "add" tool
  QToolButton* button = NULL;
  QObject* child;
  while ( (child = childIter.current()) )
  {
    button = static_cast<QToolButton*>(child);

    if ( button->textLabel() == "&Add" ) break;
    else button = NULL;

    ++childIter;
  }

  delete childList;

  if (!button)
  {
    std::cerr << "Could not find 'Add' tool" << std::endl;
    return false;
  }

  // activate the "add" tool
  if (!button->isOn()) // if not yet pressed, then press it now (otherwise, DONT press because that would deactivate the tool)
  {
    QmitkUserInputSimulation::MouseClick( button, Qt::LeftButton );
  }

  QWidget* sliceWidget;
  for (unsigned int window = 1; window < 4; ++window)
  {
    switch (window)
    {
      case 1:
        sliceWidget = m_MultiWidget->GetRenderWindow1(); // IMPORTANT to send the events to the renderwindow and not to the multiwidget or one selectableglwidget
        break;
      case 2:
        sliceWidget = m_MultiWidget->GetRenderWindow2();
        break;
      case 3:
      default:
        sliceWidget = m_MultiWidget->GetRenderWindow3();
        break;
    }

    // activate interpolation
    m_Controls->grpInterpolation->setChecked( true );
    m_Controls->m_SlicesInterpolator->EnableInterpolation( true );

    // draw a full circle
    std::cout << " circle " << std::flush;
    QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.5, 0.5, 0.1 );

    // scroll a bit
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, +1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, +1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, +1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, +1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, +1 );
    
    // draw another circle
    std::cout << " circle " << std::flush;
    QmitkUserInputSimulation::MouseDrawCircle( sliceWidget, Qt::LeftButton, 0.5, 0.5, 0.2 );
    
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, +1 ); // one behind everything now
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 ); // on first slice now
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 ); // somewhere inbetween
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 );
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 ); // on last slice now
    QmitkUserInputSimulation::MouseMoveScrollWheel( sliceWidget, -1 ); // before everything now
    
    // deactivate interpolation
    m_Controls->grpInterpolation->setChecked( false );
    m_Controls->m_SlicesInterpolator->EnableInterpolation( false );

  }

  std::cout << "[PASSED]" << std::endl;

  return true;
}


void QmitkSliceBasedSegmentation::TestOnNewSegmentationDialogFound( QWidget* widget )
{
  // method is called after the "new segmentation dialog" shows up. the dialog is found in "widget"
  if (!widget) return;

  QmitkNewSegmentationDialog* newDialog =dynamic_cast<QmitkNewSegmentationDialog*>( widget );
  if (!newDialog) return;

  QmitkMessageBoxHelper messageBoxHelper(this);

  // we simulate that some other organ is selected (click center of list), something is entered for the name, and then OK is clicked

  QmitkUserInputSimulation::MouseClick( messageBoxHelper.FindDialogItem("lstOrgans", newDialog), Qt::LeftButton ); // somewhere - should always be ok

  QLineEdit* edtName = dynamic_cast<QLineEdit*>(  messageBoxHelper.FindDialogItem("edtName", newDialog) );
  if (edtName)
  {
    QmitkUserInputSimulation::MouseClick( edtName, Qt::LeftButton ); 
    QmitkUserInputSimulation::MouseClick( edtName, Qt::LeftButton );
    QmitkUserInputSimulation::SimulateKeyboardTyping( edtName, "testname" );
  }

  QmitkUserInputSimulation::MouseClick( messageBoxHelper.FindDialogItem("btnOk", newDialog), Qt::LeftButton );
}


#endif

