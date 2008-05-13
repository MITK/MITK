#include "QmitkRegisterClasses.h"
#include "Step8.h"

#include <qapplication.h>
#include <itksys/SystemTools.hxx>
int main(int argc, char* argv[])
{
  QApplication qtapplication( argc, argv );

  if(argc<2)
  {
    fprintf( stderr, "Usage:   %s [filename1] [filename2] ...\n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str() );
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  Step8 mainWindow(argc, argv, NULL, "mainwindow");
  mainWindow.Initialize();
  qtapplication.setMainWidget(&mainWindow);
  mainWindow.show();

  // for testing
  #include "QtTesting.h"
  if(strcmp(argv[argc-1], "-testing")!=0)
    return qtapplication.exec();
  else
    return QtTesting();
}

/**
\example Step8main.cpp
*/
