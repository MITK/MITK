/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "berryProvisioningInfo.h"

#include <berryLog.h>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>

namespace berry {

#ifdef CMAKE_INTDIR
const QString ProvisioningInfo::intermediateOutDir = QString(CMAKE_INTDIR);
#else
const QString ProvisioningInfo::intermediateOutDir = QString();
#endif

ProvisioningInfo::ProvisioningInfo(const QString& file)
{
  this->readProvisioningFile(file);
}

QStringList ProvisioningInfo::getPluginDirs() const
{
  return pluginDirs.toList();
}

QList<QUrl> ProvisioningInfo::getPluginsToInstall() const
{
  return pluginsToInstall;
}

QList<QUrl> ProvisioningInfo::getPluginsToStart() const
{
  return pluginsToStart;
}

void ProvisioningInfo::readProvisioningFile(const QString& filePath)
{
  QFile file(filePath);
  file.open(QFile::ReadOnly);
  QTextStream fileStream(&file);
  QRegExp sep("\\s+");
  QString line;
  int count = 1;
  do {
    line = fileStream.readLine().trimmed();
    if (!line.isEmpty() && !line.startsWith('#'))
    {
      QString keyword = line.section(sep, 0, 0);
      QString value = line.mid(keyword.size()).trimmed();
      value = substituteKeywords(value);

      if (keyword.isEmpty())
      {
        BERRY_WARN << "Keyword missing in line " << count
                   << " of provisioning file " << filePath.toStdString();
        continue;
      }

      Keyword key = UNKNOWN;
      if (keyword.compare("READ", Qt::CaseInsensitive) == 0)
      {
        key = READ;
      }
      else if (keyword.compare("INSTALL", Qt::CaseInsensitive) == 0)
      {
        key = INSTALL;
      }
      else if (keyword.compare("START", Qt::CaseInsensitive) == 0)
      {
        key = START;
      }
      else if (keyword.compare("STOP", Qt::CaseInsensitive) == 0)
      {
        key = STOP;
      }

      if (key == UNKNOWN)
      {
        BERRY_WARN << "Keyword " << keyword.toStdString() << " in line "
                      << count << " of provisioning file "
                      << filePath.toStdString() << " unknown";
        continue;
      }

      if (value.isEmpty())
      {
        BERRY_WARN << "Value after keyword " << keyword.toStdString()
                  << " missing in line " << count
                  << " of provisioning file " << filePath.toStdString();
        continue;
      }

      switch (key)
      {
      case READ:
      {
        QUrl readFileUrl(value);
        if (!readFileUrl.isValid())
        {
          BERRY_WARN << "The READ URL " << value.toStdString()
                     << "is invalid: " << readFileUrl.errorString().toStdString();
          break;
        }
        this->readProvisioningFile(readFileUrl.toLocalFile());
        break;
      }
      case INSTALL:
      {
        this->addPluginToInstall(value);
        break;
      }
      case START:
      {
        this->addPluginToStart(value);
        break;
      }
      case STOP:
      {
        break;
      }
      }
    }
    ++count;
  } while (!line.isNull());
}

QUrl ProvisioningInfo::addPluginToInstall(const QString& file)
{
  QUrl pluginUrl(file);
  if (!pluginUrl.isValid())
  {
    BERRY_WARN << "The plugin URL " << file.toStdString() << " is invalid:"
               << pluginUrl.errorString().toStdString();
    return QUrl();
  }

  QFileInfo fileInfo(pluginUrl.toLocalFile());
  if (!fileInfo.exists())
  {
    QString fileName = fileInfo.fileName();
    QString filePath = fileInfo.absolutePath();
    if (!intermediateOutDir.isEmpty())
    {
      // search in the intermediate output dir
      QString filePath2 = filePath + "/" + intermediateOutDir;
      fileInfo = QFileInfo(filePath2 + "/" + fileName);
      if (!fileInfo.exists())
      {
        BERRY_WARN << "The plugin " << fileName.toStdString() << " was not found in "
                   << filePath.toStdString() << " or " << filePath2.toStdString();
        return QUrl();
      }
      pluginUrl = QUrl::fromLocalFile(fileInfo.canonicalFilePath());
      pluginDirs.insert(fileInfo.canonicalPath());
    }
    else
    {
      BERRY_WARN << "The plugin " << fileName.toStdString() << " was not found in "
                 << filePath.toStdString();
      return QUrl();
    }
  }
  else
  {
    pluginDirs.insert(fileInfo.canonicalPath());
  }

  pluginsToInstall.append(pluginUrl);
  return pluginUrl;
}

void ProvisioningInfo::addPluginToStart(const QString& file)
{
  QUrl pluginUrl = this->addPluginToInstall(file);
  if (!pluginUrl.isEmpty())
  {
    pluginsToStart.append(pluginUrl);
  }
}

QString ProvisioningInfo::substituteKeywords(const QString& value) const
{
  QString appPath = QCoreApplication::applicationDirPath();
  if (appPath.endsWith('/'))
  {
    appPath.chop(1);
  }

#ifdef CMAKE_INTDIR
  // Strip the intermediate dir from the application path
  QString intDir(CMAKE_INTDIR);
  if (appPath.endsWith(intDir))
  {
    appPath.chop(intDir.size()+1);
  }
#endif

#ifdef _WIN32
  if (value.contains("@EXECUTABLE_DIR") && value.contains("blueberry_osgi"))
  {
    // special case for org_blueberry_osgi in install trees for Windows
    return QString(value).replace("@EXECUTABLE_DIR", appPath, Qt::CaseInsensitive).replace("plugins/", "");
  }
#endif

  return QString(value).replace("@EXECUTABLE_DIR", appPath, Qt::CaseInsensitive);
}

}
