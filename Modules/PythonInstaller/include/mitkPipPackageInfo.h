/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPipPackageInfo_h
#define mitkPipPackageInfo_h

#include <MitkPythonInstallerExports.h>

#include <QList>
#include <QString>
#include <QStringList>

namespace mitk
{
  /** \brief Status of a single package during installation. */
  enum class PackageStatus
  {
    Pending,
    Installing,
    Installed,
    Failed
  };

  /** \brief Describes a single resolved pip package. */
  struct MITKPYTHONINSTALLER_EXPORT PipPackageInfo
  {
    QString name;       /**< Package name, e.g. "torch". */
    QString version;    /**< Resolved version, e.g. "2.8.0". */
    QString specifier;  /**< Original requirement, e.g. "torch>=2.8.0,<2.9.0".
                             Empty for transitive dependencies. */
    bool requested;     /**< True if explicitly requested, false if transitive. */
    int group;          /**< Index of the install group this package belongs to. */
  };

  /** \brief A set of packages that share the same pip options.
   *
   * Groups are resolved and installed in order. Use separate groups for
   * packages that require different index URLs (e.g. PyTorch CUDA wheels
   * vs. standard PyPI packages).
   */
  struct MITKPYTHONINSTALLER_EXPORT PipInstallGroup
  {
    QStringList requirements; /**< Package specifiers, e.g. {"torch>=2.8.0,<2.9.0"}. */
    QString indexUrl;         /**< Optional pip --index-url. Empty uses default PyPI. */
    QStringList extraPipArgs; /**< Additional pip arguments, e.g. {"--no-cache-dir"}. */
  };

  /** \brief Full specification for a pip installation run.
   *
   * Contains one or more install groups that are processed sequentially,
   * plus global options like whether to upgrade pip first.
   */
  struct MITKPYTHONINSTALLER_EXPORT PipInstallSpec
  {
    QString name;                  /**< Human-readable display name, e.g. "nnInteractive". */
    QList<PipInstallGroup> groups; /**< Install groups, processed in order. */
    bool upgradePipFirst = true;   /**< Whether to upgrade pip before installing. */
  };
}

#endif
