/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: SampleAppMain.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "SampleApp.h"
#include <qapplication.h>
#include "itkTextOutput.h"
#include <itksys/SystemTools.hxx>
#include <stdexcept>

#include <mitkStandardFileLocations.h>
#include <mitkDataStorage.h>

#include <QmitkFunctionalityTesting.h>
#include <sstream>

#include "MITKSplashScreen_xpm.h" // qembed --images MITKSplashScreen.png > MITKSplashScreen_xpm.h
#include "QmitkSplashScreen.h"
#include <qtimer.h>

/*
* BUG: ATI-grafics-cards are slow in building up display lists in case of displaying transparent surfaces.
* Activating GlobalImmediateModeRendering in vtk solves this problem.
*/
//#include <vtkMapper.h>

#include <QmitkControlsRightFctLayoutTemplate.h>

int main(int argc, char* argv[])
{
  int result;
  try
  {
    // Add directory in which the application executable resides as search directory
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(itksys::SystemTools::GetFilenamePath(argv[0]).c_str()); 
    //
    // determine application name without path and extension
    //
    std::string executableName = itksys::SystemTools::GetFilenameWithoutExtension( itksys::SystemTools::GetFilenameName(argv[0] ) );
    std::stringstream caption;
    caption << executableName << ", Builddate " << __DATE__ << ", " << __TIME__ << ".";
    itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());
    // parse testing parameters
    const char * testingParameter = NULL;
    int testType = 0;
    for (int i = 0; i < argc; i++)
    {
        if ((strcmp(argv[i], "-testing") == 0))
        {
             testType |= QmitkFunctionalityTesting::GUITest;
        }
        if ((strcmp(argv[i], "-optiontesting") == 0))
        {
             testType |= QmitkFunctionalityTesting::OptionsTest;
        }
        if ((strcmp(argv[i], "-alltesting") == 0))
        {
             testType |= QmitkFunctionalityTesting::AllTests;
        }
    }
    
    QApplication a( argc, argv );

    // popup a splash screen
    bool showSplashScreen(false);
    QmitkSplashScreen* splasher(NULL);
    if (showSplashScreen)
    {
      QImage icon = qembed_findImage("MITKSplashScreen");
      QPixmap pixmap(icon);
      splasher = new QmitkSplashScreen( pixmap );
      a.processEvents();
    }

    SampleApp mainWindow(NULL, "mainwindow",Qt::WType_TopLevel,testingParameter);
    mainWindow.setCaption( caption.str().c_str() );
    a.setMainWidget(&mainWindow);

#ifdef USEDARKPALETTE
    std::cout << "changing palette ..." << std::endl;
    QPalette p( QColor( 64,64,64), QColor(64,64,64));
    a.setPalette(p,TRUE);
#endif
    /*
    * BUG: ATI-grafics-cards are slow in building up display lists in case of displaying transparent surfaces.
    * Activating GlobalImmediateModeRendering in vtk solves this problem. Also add #include vtkMapper above
    */
    //vtkMapper::GlobalImmediateModeRenderingOn();

    mainWindow.SetDefaultWidgetSize();

    // reinit views after mainwindow and fctwidget initialization
    mainWindow.viewReinitMultiWidget();

    if (showSplashScreen)
    {
      QTimer::singleShot(5000, splasher, SLOT(close()) );
    }

    if(testType)
    {
      std::cout.setf(std::ios_base::unitbuf);
      result = StartQmitkFunctionalityTesting(mainWindow.GetFctMediator(), testType);
    }
    else
    {
      result = a.exec();
    }
  }
  catch (const std::bad_alloc& e)
  {
    std::cout << "MITK MainApp caught bad_alloc: Could not acquire enough memory: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (const std::exception& e)
  {
    std::cout << "MITK MainApp caught an exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "MITK MainApp caught something like an exception..." << std::endl;
    return EXIT_FAILURE;
  }  
  try
  {
    mitk::DataStorage::GetInstance()->ShutdownSingleton();
  }
  catch (const std::exception& e)
  {
    std::cout << "MITK MainApp caught an exception on ShutdownSingleton: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "MITK MainApp caught something like an exception on ShutdownSingleton..." << std::endl;
    return EXIT_FAILURE;
  }  
  return result;
} 
