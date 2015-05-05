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


#ifndef MITKPROVISIONINGINFO_H
#define MITKPROVISIONINGINFO_H

#include <QString>
#include <QStringList>
#include <QSet>
#include <QUrl>

namespace mitk {

class ProvisioningInfo
{
public:
    ProvisioningInfo(const std::string& file);

    QStringList getPluginDirs() const;
    QList<QUrl> getPluginsToInstall() const;
    QList<QUrl> getPluginsToStart() const;

private:

    enum Keyword {
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

    void readProvisioningFile(const std::string& file);
    QUrl addPluginToInstall(const QString& file);
    void addPluginToStart(const QString& file);

    QString substituteKeywords(const QString& value) const;
};

}

#endif // MITKPROVISIONINGINFO_H
