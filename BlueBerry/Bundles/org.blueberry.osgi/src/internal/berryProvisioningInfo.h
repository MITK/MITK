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


#ifndef BERRYPROVISIONINGINFO_H
#define BERRYPROVISIONINGINFO_H

#include <QString>
#include <QStringList>
#include <QSet>
#include <QUrl>

namespace berry {

class ProvisioningInfo
{
public:
    ProvisioningInfo(const QString& file);

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

    void readProvisioningFile(const QString& file);
    QUrl addPluginToInstall(const QString& file);
    void addPluginToStart(const QString& file);

    QString substituteKeywords(const QString& value) const;
};

}

#endif // BERRYPROVISIONINGINFO_H
