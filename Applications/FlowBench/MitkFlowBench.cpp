/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseApplication.h>

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
const QString FlowApplication::ARG_OUTPUTFORMAT = "flow.outputextension";

int main(int argc, char* argv[])
{
  FlowApplication app(argc, argv);

  app.setSingleMode(true);
  app.setApplicationName("MITK FlowBench");
  app.setOrganizationName("DKFZ");
  app.setProperty(mitk::BaseApplication::PROP_PRODUCT, "org.mitk.gui.qt.flowapplication.workbench");

  return app.run();
}
