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

#include <mitkBaseApplication.h>

#include <QVariant>


class FlowApplication : public mitk::BaseApplication
{
public:
  static const QString ARG_OUTPUTDIR;
  static const QString ARG_OUTPUTFORMAT;

  FlowApplication(int argc, char **argv) : mitk::BaseApplication(argc, argv)
  {
  };

  ~FlowApplication() = default;

protected:
  /**
  * Define command line arguments
  * @param options
  */
  void defineOptions(Poco::Util::OptionSet &options) override
  {
    Poco::Util::Option outputDirOption(ARG_OUTPUTDIR.toStdString(), "", "the location for storing persistent application data");
    outputDirOption.argument("<dir>").binding(ARG_OUTPUTDIR.toStdString());
    options.addOption(outputDirOption);

    Poco::Util::Option outputFormatOption(ARG_OUTPUTFORMAT.toStdString(), "", "the location for storing persistent application data");
    outputFormatOption.argument("<format>").binding(ARG_OUTPUTFORMAT.toStdString());
    options.addOption(outputFormatOption);

    mitk::BaseApplication::defineOptions(options);
  };
};

const QString FlowApplication::ARG_OUTPUTDIR = "flow.outputdir";
const QString FlowApplication::ARG_OUTPUTFORMAT = "flow.outputformat";

int main(int argc, char **argv)
{
  FlowApplication app(argc, argv);

  app.setSingleMode(true);
  app.setApplicationName("MITK FlowBench");
  app.setOrganizationName("DKFZ");

  // Preload the org.mitk.gui.qt.application plug-in to speed
  // up a clean-cache start. This also works around bugs in older gcc and glibc implementations,
  // which have difficulties with multiple dynamic opening and closing of shared libraries with
  // many global static initializers. It also helps if dependent libraries have weird static
  // initialization methods and/or missing de-initialization code.
  QStringList preloadLibs;
  preloadLibs << "org_mitk_gui_qt_application";
  app.setPreloadLibraries(preloadLibs);

  app.setProperty(mitk::BaseApplication::PROP_PRODUCT, "org.mitk.gui.qt.flowapplication.workbench");

  // Run the workbench.
  return app.run();
}
