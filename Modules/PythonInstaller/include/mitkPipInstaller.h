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
   * \note This class is not thread-safe. Construct, use, and destroy from a
   * single thread (typically the GUI thread). All Qt signal connections use
   * direct dispatch unless the consumer explicitly requests queued connections.
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
     * Must be called before StartInstall().
     * Has no effect (and logs a warning) while an operation is running.
     */
    void SetInstallSpec(const PipInstallSpec& spec);

    /** \brief Start the installation.
     *
     * For each install group in PipInstallSpec::groups, the engine first
     * runs a \c pip \c install \c --dry-run \c --report to determine the
     * full set of packages, then installs them one-by-one with
     * \c --no-deps. Groups are processed sequentially so that the next
     * group's resolve sees the previous group's packages already installed
     * (essential for cases like a CUDA-specific PyTorch index).
     *
     * Has no effect if an operation is already running.
     */
    void StartInstall();

    /** \brief Cancel the current operation.
     *
     * Kills the running pip process and returns immediately. The actual
     * cleanup (venv removal if one was created, InstallFinished(false)
     * emission) happens asynchronously when the killed process finishes.
     */
    void Cancel();

    /** \brief Tear down anything this installer created during a failed install.
     *
     * Intended to be called by the consumer when it gives up after a failed
     * install (e.g. the user dismisses the install dialog instead of
     * retrying). If a venv was created by this installer and the last
     * operation ended in the Failed state, the venv is removed.
     *
     * Has no effect after a successful install (Done state) or while an
     * operation is still running - in the latter case call Cancel() instead.
     */
    void AbandonInstall();

    /** \brief Whether an operation is currently running. */
    bool IsRunning() const;

    /** \brief The full list of resolved packages across all groups.
     *
     * Populated after ResolveFinished() is emitted.
     */
    const std::vector<PipPackageInfo>& GetResolvedPackages() const;

  signals:
    /** \brief Emitted when virtual environment creation starts. */
    void VirtualEnvCreationStarted();

    /** \brief Emitted when virtual environment creation finishes.
     *
     * \param[in] success Whether creation succeeded.
     */
    void VirtualEnvCreationFinished(bool success);

    /** \brief Emitted when the pip upgrade step starts. */
    void PipUpgradeStarted();

    /** \brief Emitted when the pip upgrade step finishes.
     *
     * \param[in] success Whether the upgrade succeeded. Failure is non-fatal.
     */
    void PipUpgradeFinished(bool success);

    /** \brief Emitted when resolution of a group starts. */
    void ResolveStarted();

    /** \brief Emitted when a group's resolution finishes.
     *
     * In a multi-group spec this fires once per group on success, with the
     * list growing as additional groups are resolved. Fires with success ==
     * false if pip's dependency resolver could not satisfy the requirements.
     * Setup failures (Python not found, etc.) surface via InstallFinished
     * instead.
     *
     * \param[in] success Whether resolution succeeded.
     * \param[in] packages The accumulated list of resolved packages so far.
     */
    void ResolveFinished(bool success, const std::vector<mitk::PipPackageInfo>& packages);

    /** \brief Emitted when a package's installation status changes.
     *
     * \param[in] index Index into the resolved packages list.
     * \param[in] name The package name.
     * \param[in] status The new status.
     */
    void PackageStatusChanged(int index, const QString& name, mitk::PackageStatus status);

    /** \brief Emitted when a Hugging Face model download starts.
     *
     * Fired once per entry in PipInstallSpec::huggingFaceDownloads, in order.
     *
     * \param[in] displayName The download's display name (repoId if none set).
     */
    void ModelDownloadStarted(const QString& displayName);

    /** \brief Emitted when a Hugging Face model download finishes.
     *
     * Fired once per entry on completion or failure. A failure here flips the
     * terminal InstallFinished to \c false but does not abort remaining
     * downloads - each model's failure is independent (if a user cancelled,
     * the Cancel path takes over before any more downloads start).
     *
     * \param[in] displayName The download's display name.
     * \param[in] success Whether the download succeeded.
     */
    void ModelDownloadFinished(const QString& displayName, bool success);

    /** \brief Emitted when all packages have been installed (or installation failed).
     *
     * If the spec includes Hugging Face downloads, this fires only after
     * those downloads have also finished.
     *
     * \param[in] success True only if every package installed successfully
     *                    and every model download succeeded.
     */
    void InstallFinished(bool success);

    /** \brief Raw process output for display in a details view.
     *
     * \param[in] text The output text.
     * \param[in] isError True if the text came from stderr.
     */
    void OutputReceived(const QString& text, bool isError);

    /** \brief Overall installation progress.
     *
     * \param[in] current Number of packages installed so far.
     * \param[in] total Total number of packages to install.
     */
    void ProgressChanged(int current, int total);

    /** \brief Emitted on fatal errors (e.g. Python executable not found). */
    void ErrorOccurred(const QString& message);

  private slots:
    void OnStandardOutputReady();
    void OnStandardErrorReady();
    void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  private:
    enum class State
    {
      Idle,
      CreatingVirtualEnv,
      UpgradingPip,
      Resolving,
      Installing,
      DownloadingModels,
      Cancelling,
      Done,
      Failed
    };

    void BeginVirtualEnvPhase();
    void StartPipUpgrade();
    void StartResolveGroup();
    void StartInstallPackage();
    void BeginModelDownloadPhase();
    void StartModelDownload();
    QStringList BuildPipArgs(const QStringList& baseArgs, const PipInstallGroup& group) const;
    bool ParseResolveReport(const QString& reportPath, int groupIndex);
    QString PythonExecutable() const;
    void FinalizeCancel();
    void RemoveCreatedVirtualEnv();

    PipInstallSpec m_Spec;
    std::vector<PipPackageInfo> m_ResolvedPackages;

    QProcess* m_Process = nullptr;
    State m_State = State::Idle;
    bool m_AnyFailed = false;
    bool m_CreatedVirtualEnv = false;

    int m_CurrentGroup = 0;
    int m_CurrentPackage = 0;
    int m_GroupStartIndex = 0;
    int m_CurrentDownload = 0;

    QTemporaryFile m_ReportFile;
  };
}

#endif
