#include <berryStarter.h>

#include <QApplication>

int main(int argc, char** argv)
{
  // Create a QApplication instance first
  QApplication myApp(argc, argv);
  myApp.setApplicationName("@CUSTOM_PROJECT_NAME@");
  myApp.setOrganizationName("@PLUGIN_VENDOR@");

  return berry::Starter::Run(argc, argv);
}
