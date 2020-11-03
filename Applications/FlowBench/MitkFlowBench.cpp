/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseApplication.h>
#include <QVariant>

#if defined __GNUC__ && !defined __clang__
#  include <QDir>
#  include <QFileInfo>
#  include <QString>
#  include <QStringList>
#endif

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

int main(int argc, char **argv)
{
  FlowApplication app(argc, argv);

  app.setSingleMode(true);
  app.setApplicationName("MITK FlowBench");
  app.setOrganizationName("DKFZ");

  // Preload the org.blueberry.core.expressions plugin to work around a bug in
  // GCC that leads to undefined symbols while loading certain libraries even though
  // the symbols are actually defined.
#if defined __GNUC__ && !defined __clang__
  auto library = QFileInfo(argv[0]).dir().path() + "/../lib/plugins/liborg_blueberry_core_expressions.so";

  if (!QFileInfo(library).exists())
    library = "liborg_blueberry_core_expressions";

  app.setPreloadLibraries(QStringList() << library);
#endif

  app.setProperty(mitk::BaseApplication::PROP_PRODUCT, "org.mitk.gui.qt.flowapplication.workbench");

  // Run the workbench.
  return app.run();
}
