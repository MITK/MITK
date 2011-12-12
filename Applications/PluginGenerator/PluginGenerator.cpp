/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "ctkCommandLineParser.h"
#include <PluginGeneratorConfig.h>

#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QBuffer>

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <cctype>

int compareVersions(const QString& v1, const QString& v2)
{
  QList<int> t1;
  foreach(QString t, v1.split('.'))
  {
    t1.append(t.toInt());
  }

  QList<int> t2;
  foreach(QString t, v2.split('.'))
  {
    t2.append(t.toInt());
  }

  // pad with 0
  if (t1.size() < t2.size())
  {
    for(int i = 0; i < t2.size() - t1.size(); ++i)
    t1.append(0);
  }
  else
  {
    for(int i = 0; i < t1.size() - t2.size(); ++i)
    t2.append(0);
  }

  for(int i = 0; i < t1.size(); ++i)
  {
    if (t1.at(i) < t2.at(i)) return -1;
    else if (t1.at(i) > t2.at(i)) return 1;
  }
  return 0;
}

int checkUpdates(QTextStream& out)
{
  out << "Checking for updates... "; out.flush();
  QNetworkAccessManager manager;
  QNetworkReply* reply = manager.get(QNetworkRequest(QUrl("http://www.mitk.org/wiki/PluginGeneratorUpdate?action=raw")));

  QEventLoop eventLoop;
  reply->connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
  eventLoop.exec();

  QByteArray data = reply->readAll();
  delete reply;

  if (data.isEmpty())
  {
    qCritical() << "A network error occured:" << reply->errorString();
    return EXIT_FAILURE;
  }

  QBuffer buffer(&data);
  buffer.open(QIODevice::ReadOnly);
  bool versionFound = false;
  while(buffer.canReadLine())
  {
    QString line = buffer.readLine();
    if (!versionFound)
    {
      line = line.trimmed();
      if (line.isEmpty()) continue;

      if (compareVersions(PLUGIN_GENERATOR_VERSION, line) < 0)
      {
        versionFound = true;
        out << "New version " << line << " available.\n";
      }
      else
      {
        // no update available
        out << "No update available.\n"; out.flush();
        return EXIT_SUCCESS;
      }
    }
    else
    {
      out << line;
    }
  }

  if (!versionFound)
  {
    qCritical() << "Update information not readable.";
    return EXIT_FAILURE;
  }

  out << '\n'; out.flush();

  return EXIT_SUCCESS;
}

bool readAnswer(char defaultAnswer)
{
  std::string line;
  std::cin >> std::noskipws >> line;

  // consume the new line character
  std::cin.clear();
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  char answer = defaultAnswer;
  if (!line.empty() && line[0] != '\n')
  {
    answer = std::tolower(line[0]);
  }

  if (answer == 'y') return true;
  if (answer == 'n') return false;
  if (defaultAnswer == 'y') return true;
  return false;
}

void createFilePathMapping(const QString& templateName, const QString& baseInDir, const QString& baseOutDir, QHash<QString, QString>& fileNameMapping)
{
  QFileInfo info(templateName);
  if (info.isDir())
  {
    QStringList subEntries = QDir(templateName).entryList();
    foreach(QString subEntry, subEntries)
    {
      createFilePathMapping(templateName + "/" + subEntry, baseInDir, baseOutDir, fileNameMapping);
    }
    return;
  }

  fileNameMapping[templateName] = QString(templateName).replace(baseInDir, baseOutDir);
}

QHash<QString,QString> createTemplateFileMapping(const QString& qrcBase, const QString& baseOutDir, const QHash<QString, QString>& fileNameMapping)
{
  QHash<QString,QString> filePathMapping;
  createFilePathMapping(qrcBase, qrcBase, baseOutDir, filePathMapping);

  QMutableHashIterator<QString,QString> i(filePathMapping);
  while(i.hasNext())
  {
    i.next();
    QHashIterator<QString,QString> j(fileNameMapping);
    while(j.hasNext())
    {
      j.next();
      i.setValue(i.value().replace(j.key(), j.value()));
    }
  }

  return filePathMapping;
}

bool generateFiles(const QHash<QString, QString>& parameters,
                   const QHash<QString, QString>& filePathMapping)
{
  QHashIterator<QString,QString> paths(filePathMapping);
  while(paths.hasNext())
  {
    paths.next();
    QFile templ(paths.key());
    templ.open(QIODevice::ReadOnly);
    QByteArray templContent = templ.readAll();

    QHashIterator<QString,QString> i(parameters);
    while (i.hasNext())
    {
      i.next();
      templContent.replace(i.key(), QByteArray(i.value().toLatin1()));
    }

    QFile outTempl(paths.value());
    QDir dir(QFileInfo(outTempl).dir());
    if (!dir.exists())
    {
      if (!dir.mkpath(dir.absolutePath()))
      {
        qCritical() << "Could not create directory" << dir.absolutePath();
        return EXIT_FAILURE;
      }
    }

    if (!outTempl.open(QIODevice::WriteOnly))
    {
      qCritical() << outTempl.errorString();
      return false;
    }
    outTempl.write(templContent);
  }

  return true;
}

int main(int argc, char** argv)
{
  QString appName("MITKPluginGenerator");

  QCoreApplication app(argc, argv);
  app.setApplicationName(appName);
  app.setOrganizationName("DKFZ");
  
  ctkCommandLineParser parser;
  // Use Unix-style argument names
  parser.setArgumentPrefix("--", "-");
  parser.setStrictModeEnabled(true);

  // Add command line argument names
  parser.addArgument("help", "h", QVariant::Bool, "  Show this help text");
  parser.addArgument("out-dir", "o", QVariant::String, "  Output directory", QDir::tempPath());
  parser.addArgument("license", "l", QVariant::String, "  Path to a file containing license information", ":/MITKLicense.txt");
  parser.addArgument("vendor", "v", QVariant::String, "  The vendor of the generated code", "DKFZ, Medical and Biological Informatics");
  parser.addArgument("quiet", "q", QVariant::Bool, "  Do not print additional information");
  parser.addArgument("confirm-all", "y", QVariant::Bool, "  Answer all questions with 'yes'");
  parser.addArgument("check-update", "u", QVariant::Bool, "  Check for updates and exit");
  parser.addArgument("no-networking", "n", QVariant::Bool, "  Disable all network requests");

  parser.beginGroup("Plug-in options");
  parser.addArgument("plugin-symbolic-name", "ps", QVariant::String, "* The plugin's symbolic name");
  parser.setExactMatchRegularExpression("-ps", "^[a-zA-Z]+\\.[a-zA-Z0-9._]+[^\\.]$", "Symbolic name invalid");
  parser.addArgument("plugin-name", "pn", QVariant::String, "  The plug-in's human readable name");

  parser.beginGroup("Plug-in View options");
  parser.addArgument("view-class", "vc", QVariant::String, "  The View's' class name");
  parser.addArgument("view-name", "vn", QVariant::String, "* The View's human readable name");

  parser.beginGroup("Project options");
  parser.addArgument("project-copyright", "", QVariant::String, "  Path to a file containing copyright information", ":/MITKCopyright.txt");
  parser.addArgument("project-name", "", QVariant::String, "  The project name");
  parser.setExactMatchRegularExpression("--project-name", "^[a-zA-Z_\\-]+$", "Project name invalid");
  parser.addArgument("project-app-name", "", QVariant::String, "  The application name");
  parser.setExactMatchRegularExpression("--project-app-name", "^[a-zA-Z_\\-]+$", "Project application name invalid");
  parser.endGroup();


  // Parse the command line arguments
  bool ok = false;
  QHash<QString, QVariant> parsedArgs = parser.parseArguments(QCoreApplication::arguments(), &ok);
  if (!ok)
  {
    QTextStream(stderr, QIODevice::WriteOnly) << "Error parsing arguments: "
     << parser.errorString() << "\nType '" << appName << " -h' for help\n";
   return EXIT_FAILURE;
  }

  QTextStream out(stdout, QIODevice::WriteOnly);
 
  // Show a help message
  if (parsedArgs.contains("help"))
  {
    out << "A CTK plug-in generator for MITK (version " PLUGIN_GENERATOR_VERSION ")\n\n"
        << parser.helpText()
        << "\n[* - options are required]\n";
    return EXIT_SUCCESS;
  }

  bool noNetwork = parsedArgs.contains("no-networking");

  if (parsedArgs.contains("check-update"))
  {
    if (noNetwork)
    {
      out << "Network support disabled. Cannot check for updates.\n";
      out.flush();
    }
    else
    {
      return checkUpdates(out);
    }
  }
  else
  {
    // always check for updates if no-networking is not given
    if (!noNetwork)
    {
      checkUpdates(out);
    }
  }
 
  // Check arguments

  // Project options
  QString projectName = parsedArgs["project-name"].toString();
  QString projectAppName = parsedArgs["project-app-name"].toString();
  QString copyrightPath = QDir::fromNativeSeparators(parsedArgs["project-copyright"].toString());

  bool createProject = !projectName.isEmpty();
  if (createProject && projectAppName.isEmpty())
  {
    projectAppName = projectName;
  }

  QString pluginSymbolicName = parsedArgs["plugin-symbolic-name"].toString();
  if (pluginSymbolicName.isEmpty())
  {
    qCritical() << "Required argument 'plugin-symbolic-name' missing.";
    qCritical("%s%s%s", "Type '", qPrintable(appName), " -h' for help");
    return EXIT_FAILURE;
  }

  QString pluginTarget(pluginSymbolicName);
  pluginTarget.replace('.', '_');

  QString activatorClass = pluginTarget + "_Activator";

  QString outDir = QDir::fromNativeSeparators(parsedArgs["out-dir"].toString());
  QString licensePath = QDir::fromNativeSeparators(parsedArgs["license"].toString());
  QString pluginExportDirective = pluginSymbolicName.split('.').last().toUpper() + "_EXPORT";
 
  QString pluginName = parsedArgs["plugin-name"].toString();
  if (pluginName.isEmpty())
  {
    QStringList toks = pluginSymbolicName.split('.');
    pluginName = toks.last();
    pluginName[0] = pluginName[0].toUpper();
  }

  QString vendor = parsedArgs["vendor"].toString();

  QString viewName = parsedArgs["view-name"].toString();
  if (viewName.isEmpty())
  {
    qCritical() << "Required argument 'view-name' missing.";
    qCritical("%s%s%s", "Type '", qPrintable(appName), " -h' for help");
    return EXIT_FAILURE;
  }

  QStringList toks = viewName.split(QRegExp("\\s"), QString::SkipEmptyParts);
  QString viewClass = parsedArgs["view-class"].toString();
  if (viewClass.isEmpty())
  {
    foreach(QString tok, toks)
    {
      QString tmp = tok;
      tmp[0] = tmp[0].toUpper();
      viewClass += tmp;
    }
  }

  QString viewId;
  if (viewId.isEmpty())
  {
    viewId = "org.mitk.views.";
    foreach(QString tok, toks)
    {
      viewId += tok.toLower();
    }
  }

  bool quiet = parsedArgs.contains("quiet");
  bool autoConfirm = parsedArgs.contains("confirm-all");

  if (!outDir.endsWith('/'))
    outDir += '/';
  if (createProject)
    outDir += projectName;
  else
    outDir += pluginSymbolicName;

  // Print the collected information
  if(!quiet)
  {
    if (createProject)
    {
      out << "Using the following information to create a project:\n\n"
          << "  Project Name:        " << projectName << '\n'
          << "  Application Name:    " << projectAppName << '\n'
          << "  Copyright File:      " << QDir::toNativeSeparators(copyrightPath) << '\n';
    }
    else
    {
      out << "Using the following information to create a plug-in:\n\n";
    }

    out << "  License File:        " << QDir::toNativeSeparators(licensePath) << '\n'
        << "  Plugin-SymbolicName: " << pluginSymbolicName << '\n'
        << "  Plugin-Name:         " << pluginName << '\n'
        << "  Plugin-Vendor:       " << vendor << '\n'
        << "  View Name:           " << viewName << '\n'
        << "  View Id:             " << viewId << '\n'
        << "  View Class:          " << viewClass << '\n' << '\n'
        << "Create in: " << outDir << '\n' << '\n';

    if (!autoConfirm)
      out << "Continue [Y/n]? ";

    out.flush();

    if(!autoConfirm && !readAnswer('y'))
    {
      out << "Aborting.\n";
      return EXIT_SUCCESS;
    }
  }

  // Check the output directory
  if (!QDir(outDir).exists())
  {
    if (!autoConfirm)
    {
      out << "Directory '" << outDir << "' does not exist. Create it [Y/n]? ";
      out.flush();
    }

    if (autoConfirm || readAnswer('y'))
    {
      if (!QDir().mkpath(outDir))
      {
        qCritical() << "Could not create directory:" << outDir;
        return EXIT_FAILURE;
      }
    }
    else
    {
      out << "Aborting.\n";
      return EXIT_SUCCESS;
    }

  }

  if (!QDir(outDir).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).isEmpty())
  {
    if (!autoConfirm)
    {
      out << "Directory '" << outDir << "' is not empty. Continue [y/N]? ";
      out.flush();
    }

    if (!autoConfirm && !readAnswer('n'))
    {
      out << "Aborting.\n";
      return EXIT_SUCCESS;
    }
  }

  // Extract the license text
  QFile licenseFile(licensePath);
  if (!licenseFile.open(QIODevice::ReadOnly))
  {
    qCritical() << "Cannot open file" << licenseFile.fileName();
    return EXIT_FAILURE;
  }
  QString licenseText = licenseFile.readAll();
  licenseFile.close();

  QHash<QString,QString> parameters;
  if (createProject)
  {
    // Extract the copyright
    QFile copyrightFile(copyrightPath);
    if (!copyrightFile.open(QIODevice::ReadOnly))
    {
      qCritical() << "Cannot open file" << copyrightFile.fileName();
      return EXIT_FAILURE;
    }
    QString copyrighText = copyrightFile.readAll();
    copyrightFile.close();

    parameters["$(copyright)"] = copyrighText;
    parameters["$(project-name)"] = projectName;
    parameters["$(project-app-name)"] = projectAppName;
    parameters["$(project-plugins)"] = QString("Plugins/") + pluginSymbolicName + ":ON";

    QStringList toks = pluginTarget.split("_");
    QString projectPluginBase = toks[0] + "_" + toks[1];
    parameters["$(project-plugin-base)"] = projectPluginBase;
  }
  parameters["$(license)"] = licenseText;
  parameters["$(plugin-name)"] = pluginName;
  parameters["$(plugin-symbolic-name)"] = pluginSymbolicName;
  parameters["$(vendor)"] = vendor;
  parameters["$(plugin-target)"] = pluginTarget;
  parameters["$(plugin-export-directive)"] = pluginExportDirective;
  parameters["$(view-id)"] = viewId;
  parameters["$(view-name)"] = viewName;
  parameters["$(view-file-name)"] = viewClass;
  parameters["$(view-class-name)"] = viewClass;
  parameters["$(activator-file-name)"] = activatorClass;
  parameters["$(activator-class-name)"] = activatorClass;

  if (createProject)
  {
    QHash<QString,QString> projectFileNameMapping;
    projectFileNameMapping["TemplateApp"] = projectAppName;

    QHash<QString,QString> filePathMapping = createTemplateFileMapping(":/ProjectTemplate", outDir, projectFileNameMapping);
    generateFiles(parameters, filePathMapping);
  }

  QHash<QString,QString> pluginFileNameMapping;
  pluginFileNameMapping["QmitkTemplateView"] = viewClass;
  pluginFileNameMapping["mitkPluginActivator"] = activatorClass;

  if (createProject)
  {
    if (!outDir.endsWith('/'))
      outDir += '/';
    outDir += "Plugins/" + pluginSymbolicName;
  }
  QHash<QString,QString> filePathMapping = createTemplateFileMapping(":/PluginTemplate", outDir, pluginFileNameMapping);
  generateFiles(parameters, filePathMapping);

  return EXIT_SUCCESS;
}
