$(license)

#include <mitkBaseApplication.h>

#include <QVariant>

int main(int argc, char **argv)
{
  // Create a QApplication instance first
  mitk::BaseApplication myApp(argc, argv);
  myApp.setApplicationName("TestApp");
  myApp.setOrganizationName("DKFZ");

  myApp.setProperty(mitk::BaseApplication::PROP_APPLICATION, "org.mitk.qt.extapplication");
  return myApp.run();
}
