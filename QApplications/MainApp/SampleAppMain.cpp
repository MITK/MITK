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

/*
* BUG: ATI-grafics-cards are slow in building up display lists in case of displaying transparent surfaces.
* Activating GlobalImmediateModeRendering in vtk solves this problem.
*/
//#include <vtkMapper.h>

#include <QmitkFunctionalityTesting.h>
#include <sstream>

int main(int argc, char* argv[])
{
  try
  {
    //
    // determine application name without path and extension
    //
    std::string executableName = itksys::SystemTools::GetFilenameWithoutExtension( itksys::SystemTools::GetFilenameName(argv[0] ) );
    std::stringstream caption;
    caption << executableName << ", Builddate " << __DATE__ << ", " << __TIME__ << ".";
    itk::OutputWindow::SetInstance(itk::TextOutput::New().GetPointer());
    QApplication a( argc, argv );
    SampleApp mainWindow(NULL, "mainwindow");
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
    mainWindow.showMaximized();
    if(strcmp(argv[argc-1], "-testing")==0) {
      std::cout.setf(std::ios_base::unitbuf);
      return StartQmitkFunctionalityTesting(mainWindow.GetFctMediator());
    }
    else
    {
      return a.exec();
    }
  }
  catch ( const std::exception & e )
  {
    std::cout << "Exception caught: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
} 

