/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPipInstaller_h
#define mitkPipInstaller_h

#include <mitkPipPackageInfo.h>

#include <QObject>
#include <QProcess>
#include <QTemporaryFile>

namespace mitk
{
  /** \brief Headless pip installer with structured progress reporting.
   *
   * Manages pip package installation as a two-phase process per install group:
   * -# **Resolve**: runs \c pip \c install \c --dry-run \c --report to determine
   *    the full set of packages that would be installed.
   * -# **Install**: installs resolved packages one-by-one with \c --no-deps
   *    for per-package status tracking.
   *
   * Install groups (PipInstallGroup) are processed sequentially. Each group
   * can have its own index URL and pip arguments. This supports scenarios like
   * installing PyTorch from a CUDA-specific index while installing other
   * packages from the default PyPI.
   *
   * All operations are asynchronous. Progress is communicated through Qt signals.
   *
   * \sa PipInstallSpec, PipInstallGroup, PipPackageInfo, QmitkPipInstallDialog
   */
  class MITKPYTHONINSTALLER_EXPORT PipInstaller : public QObject
  {
    Q_OBJECT

  public:
    explicit PipInstaller(QObject* parent = nullptr);
    ~PipInstaller() override;

    /** \brief Set the installation specification.
     *
     * Must be called before StartResolve() or StartResolveAndInstall().
     */
    void SetInstallSpec(const PipInstallSpec& spec);

    /** \brief Start resolving dependencies for all groups.
     *
     * Upgrades pip first if PipInstallSpec::upgradePipFirst is true.
     * Emits resolveFinished() with the accumulated package list once
     * all groups have been resolved.
     */
    void StartResolve();

    /** \brief Start installing all previously resolved packages.
     *
     * Call after resolveFinished() has been emitted with success.
     * Installs packages one-by-one, emitting packageStatusChanged()
     * for each.
     */
    void StartInstall();

    /** \brief Convenience: resolve all groups, then install automatically. */
    void StartResolveAndInstall();

    /** \brief Cancel the current operation.
     *
     * Kills the running pip process. Emits installFinished(false) or
     * resolveFinished(false) depending on the current phase.
     */
    void Cancel();

    /** \brief Whether an operation is currently running. */
    bool IsRunning() const;

    /** \brief The full list of resolved packages across all groups.
     *
     * Populated after resolveFinished() is emitted.
     */
    QList<PipPackageInfo> ResolvedPackages() const;

  signals:
    /** \brief Emitted when the pip upgrade step starts. */
    void pipUpgradeStarted();

    /** \brief Emitted when the pip upgrade step finishes.
     *
     * \param[in] success Whether the upgrade succeeded. Failure is non-fatal.
     */
    void pipUpgradeFinished(bool success);

    /** \brief Emitted when resolution of a group starts. */
    void resolveStarted();

    /** \brief Emitted when all groups have been resolved (or resolution failed).
     *
     * \param[in] success Whether resolution succeeded.
     * \param[in] packages The accumulated list of resolved packages.
     */
    void resolveFinished(bool success, const QList<mitk::PipPackageInfo>& packages);

    /** \brief Emitted when a package's installation status changes.
     *
     * \param[in] index Index into the resolved packages list.
     * \param[in] name The package name.
     * \param[in] status The new status.
     */
    void packageStatusChanged(int index, const QString& name, mitk::PackageStatus status);

    /** \brief Emitted when all packages have been installed (or installation failed).
     *
     * \param[in] success True only if every package installed successfully.
     */
    void installFinished(bool success);

    /** \brief Raw process output for display in a details view.
     *
     * \param[in] text The output text.
     * \param[in] isError True if the text came from stderr.
     */
    void outputReceived(const QString& text, bool isError);

    /** \brief Overall installation progress.
     *
     * \param[in] current Number of packages installed so far.
     * \param[in] total Total number of packages to install.
     */
    void progressChanged(int current, int total);

    /** \brief Emitted on fatal errors (e.g. Python executable not found). */
    void errorOccurred(const QString& message);

  private slots:
    void OnStandardOutputReady();
    void OnStandardErrorReady();
    void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  private:
    enum class State
    {
      Idle,
      UpgradingPip,
      Resolving,
      Installing,
      Done,
      Failed
    };

    void StartPipUpgrade();
    void StartResolveGroup();
    void StartInstallPackage();
    QStringList BuildPipArgs(const QStringList& baseArgs, const PipInstallGroup& group) const;
    bool ParseResolveReport(const QString& reportPath, int groupIndex);
    QString PythonExecutable() const;

    PipInstallSpec m_Spec;
    QList<PipPackageInfo> m_ResolvedPackages;

    QProcess* m_Process = nullptr;
    State m_State = State::Idle;
    bool m_AutoInstall = false;
    bool m_AnyFailed = false;

    int m_CurrentGroup = 0;
    int m_CurrentPackage = 0;
    int m_GroupStartIndex = 0;

    QTemporaryFile m_ReportFile;
  };
}

#endif
