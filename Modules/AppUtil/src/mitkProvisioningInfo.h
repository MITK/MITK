/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPROVISIONINGINFO_H
#define MITKPROVISIONINGINFO_H

#include <QSet>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace mitk
{
  class ProvisioningInfo
  {
  public:
    ProvisioningInfo(const QString &file);

    QStringList getPluginDirs() const;
    QList<QUrl> getPluginsToInstall() const;
    QList<QUrl> getPluginsToStart() const;

  private:
    enum Keyword
    {
      UNKNOWN,
      READ,
      INSTALL,
      START,
      STOP
    };

    QSet<QString> pluginDirs;
    QList<QUrl> pluginsToInstall;
    QList<QUrl> pluginsToStart;

    static const QString intermediateOutDir;

    void readProvisioningFile(const QString &file);
    QUrl addPluginToInstall(const QString &file);
    void addPluginToStart(const QString &file);

    QString substituteKeywords(const QString &value) const;
  };
}

#endif // MITKPROVISIONINGINFO_H
