#include <Step6.h>

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

  Step6 mainWindow(argc, argv, NULL, "mainwindow");
  qtapplication.setMainWidget(&mainWindow);
  mainWindow.show();

  return qtapplication.exec();
}
