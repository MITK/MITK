/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkProvisioningInfo_h
#define mitkProvisioningInfo_h

#include <QSet>
#include <QString>
#include <QStringList>
#include <QUrl>

namespace mitk
{
  /**
   * \brief Parses a provisioning file to determine which plugins to install and start.
   *
   * The provisioning file uses a simple text format with keywords READ, INSTALL, START,
   * and STOP followed by plugin URLs. The special placeholder \@EXECUTABLE_DIR is
   * substituted with the application's directory path.
   */
  class ProvisioningInfo
  {
  public:
    /**
     * \brief Construct a ProvisioningInfo by parsing the given provisioning file.
     *
     * \param file Path to the provisioning file.
     * \throw mitk::Exception if the file cannot be opened.
     */
    ProvisioningInfo(const QString &file);

    /** \brief Return the list of directories containing discovered plugins. */
    QStringList getPluginDirs() const;

    /** \brief Return the list of plugin URLs that should be installed. */
    QList<QUrl> getPluginsToInstall() const;

    /** \brief Return the list of plugin URLs that should be started after installation. */
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

#endif
