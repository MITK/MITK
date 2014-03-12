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

#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include <mitkLog.h>

#include "mitkGlobalInteraction.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkDataNodeFactory.h"
#include "mitkDisplayInteractor.h"
#include "QmitkRenderWindow.h"
#include <QApplication>

#include "QmitkRegisterClasses.h"

#include <QTest>
#include <QTestEventList>

#include "usModuleRegistry.h"
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include "usServiceProperties.h"
#include "mitkInteractionEventObserver.h"

#include "itkTimeProbe.h"

class QmitkInteractionPerformanceTest
{

  public:

    static QmitkRenderWindow* InitializeGUI()
    {
      QmitkRegisterClasses();

      mitk::DataStorage::Pointer storage = mitk::RenderingManager::GetInstance()->GetDataStorage();
      if ( storage.IsNull() )
      {
        storage = mitk::StandaloneDataStorage::New();
        mitk::RenderingManager::GetInstance()->SetDataStorage( storage );
      }

      QmitkRenderWindow* rw = new QmitkRenderWindow();
      rw->show();
      rw->setFixedHeight(600);
      rw->setFixedWidth(600);

      return rw;
    }

    static mitk::DataNode::Pointer LoadImage()
    {
      mitk::DataNodeFactory::Pointer nodeFac = mitk::DataNodeFactory::New();
      nodeFac->SetFileName( "D:/dicom.dcm" );
      nodeFac->Update();
      mitk::DataNode::Pointer node = nodeFac->GetOutput();

      mitk::DataStorage::Pointer storage = mitk::RenderingManager::GetInstance()->GetDataStorage();
      storage->Add( node );

      mitk::Geometry3D::Pointer geo = NULL;
      geo = node->GetData()->GetGeometry();
      mitk::RenderingManager::GetInstance()->InitializeViews( geo );
      //mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();

      return node;
    }

    static void MoveMouseOnRenderWindow( QWidget* window, QPoint startPoint, QPoint endpoint )
    {
      QPoint modifier = endpoint;
      modifier -= startPoint;
     // modifier /= 4;

      QTest::mousePress( window, Qt::LeftButton, NULL, startPoint );

      // move the cursor.
      // This has to be done by creating QMouseEvents manually as QTest::mouseMove does not work.
      // see QT-Bug #5232 at http://bugreports.qt.nokia.com/browse/QTBUG-5232?page=com.atlassian.jira.plugin.system.issuetabpanels%3Aall-tabpanel
      //
      // furthermore, as one moveEvent can only 'cause' a zoom-factor of 0.05 we should do this at least three times to achieve
      // at least a little zooming
      QMouseEvent* moveEvent = new QMouseEvent( QEvent::MouseMove, startPoint+=modifier, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
      QApplication::sendEvent( window, moveEvent );

      QMouseEvent* moveEvent2 = new QMouseEvent( QEvent::MouseMove, startPoint+=modifier, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
      QApplication::sendEvent( window, moveEvent2 );

      QTest::mouseRelease( window, Qt::LeftButton, NULL, endpoint );
    }


protected:
private:
};


int QmitkInteractionPerformanceTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("QmitkInteractionPerformanceTest");

  QApplication* app = new QApplication(argc, argv);


  QmitkRenderWindow* renderWindow = QmitkInteractionPerformanceTest::InitializeGUI();
  mitk::DataNode::Pointer node = QmitkInteractionPerformanceTest::LoadImage();

  mitk::DisplayInteractor::Pointer interactor = mitk::DisplayInteractor::New();
  interactor->LoadStateMachine( "DisplayInteraction.xml" );
  interactor->SetEventConfig( "DisplayConfigPACSLevelWindow.xml" );

  us::ServiceProperties props;
  props["name"] = std::string("Interactor");
  us::ModuleRegistry::GetModule(1)->GetModuleContext()->RegisterService<mitk::InteractionEventObserver>( interactor.GetPointer(),props);

  itk::TimeProbe timer;

  mitk::LevelWindow startLw;
  node->GetLevelWindow(startLw);

  for( int loops=0; loops<10; loops++ )
  {
    node->SetLevelWindow(startLw);

    timer.Start();
    for( int i=1; i<100; i++ )
    {
      QmitkInteractionPerformanceTest::MoveMouseOnRenderWindow( renderWindow, QPoint(i,i), QPoint(i+6,i+6) );
      //mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
    }
    timer.Stop();

  }

  MITK_INFO << "Took me " << timer.GetMean();

  //app->exec();


  // always end with this!
  MITK_TEST_END();
}
