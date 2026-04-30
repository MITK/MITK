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

#include <nlohmann/json_fwd.hpp>

#include <string>
#include <vector>

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
    std::string name;       /**< Package name, e.g. "torch". */
    std::string version;    /**< Resolved version, e.g. "2.8.0". */
    std::string specifier;  /**< Original requirement, e.g. "torch>=2.8.0,<2.9.0".
                                 Empty for transitive dependencies. */
    bool requested = false; /**< True if explicitly requested, false if transitive. */
    int group = 0;          /**< Index of the install group this package belongs to.
                                 Indexes the installer's working list, which may include
                                 an implicit trailing group for huggingface_hub when the
                                 spec carries Hugging Face downloads. */
  };

  /** \brief A set of packages that share the same pip options.
   *
   * Groups are resolved and installed in order. Use separate groups for
   * packages that require different index URLs (e.g. PyTorch CUDA wheels
   * vs. standard PyPI packages).
   *
   * \code
   * // Simple: just requirements (default PyPI)
   * PipInstallGroup group({ "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" });
   *
   * // With a custom index URL
   * group.indexUrl = "https://download.pytorch.org/whl/cu128";
   * \endcode
   */
  struct MITKPYTHONINSTALLER_EXPORT PipInstallGroup
  {
    PipInstallGroup() = default;

    PipInstallGroup(std::initializer_list<std::string> requirements);

    std::vector<std::string> requirements; /**< Package specifiers, e.g. {"torch>=2.8.0,<2.9.0"}. */
    std::string indexUrl;                  /**< Optional pip --index-url. Empty uses default PyPI. */
    std::vector<std::string> extraPipArgs; /**< Additional pip arguments, e.g. {"--no-cache-dir"}. */
  };

  /// \brief Files to fetch from a Hugging Face repository after pip install.
  ///
  /// Processed by PipInstaller as a terminal phase once all pip groups have
  /// been installed successfully. The download runs `huggingface_hub`'s
  /// \c snapshot_download in the activated venv, so the venv must contain
  /// \c huggingface_hub (typically a transitive dependency of an ML package
  /// installed in one of the groups).
  ///
  /// \code
  /// HuggingFaceDownload model;
  /// model.repoId = "nnInteractive/nnInteractive";
  /// model.allowPatterns = { "nnInteractive_v1.0/*" };
  /// model.displayName = "nnInteractive model";
  /// spec.huggingFaceDownloads.push_back(std::move(model));
  /// \endcode
  ///
  struct MITKPYTHONINSTALLER_EXPORT HuggingFaceDownload
  {
    std::string repoId;                     /**< Hugging Face repo id, e.g. "nnInteractive/nnInteractive". */
    std::vector<std::string> allowPatterns; /**< Glob patterns to restrict the download. Empty = whole repo. */
    std::string displayName;                /**< Label for the installer UI. Falls back to \c repoId if empty. */
    bool optional = false;                  /**< If true, a download failure does not fail the overall installation. */
  };

  /** \brief Full specification for a pip installation run.
   *
   * Contains one or more install groups that are processed sequentially,
   * plus global options like whether to upgrade pip first, plus an optional
   * list of Hugging Face repositories to fetch after pip install completes.
   */
  struct MITKPYTHONINSTALLER_EXPORT PipInstallSpec
  {
    std::string name;                      /**< Human-readable display name, e.g. "nnInteractive". */
    std::string venvName;                  /**< If set, create and activate this venv before installing. */
    std::vector<PipInstallGroup> groups;   /**< Install groups, processed in order. */
    bool upgradePipFirst = true;           /**< Whether to upgrade pip before installing. */
    std::vector<HuggingFaceDownload> huggingFaceDownloads; /**< Model weights to fetch after pip install. */

    /** \brief Load a PipInstallSpec from a JSON file.
     *
     * \param[in] path File system path to a .json file.
     * \return The deserialized install spec.
     * \throws std::runtime_error If the file cannot be read or parsed.
     */
    static PipInstallSpec FromFile(const std::string& path);

    /** \brief Load a PipInstallSpec from a Qt resource path.
     *
     * \param[in] resourcePath Qt resource path, e.g. ":/nnInteractive/install_spec.json".
     * \return The deserialized install spec.
     * \throws std::runtime_error If the resource cannot be read or parsed.
     */
    static PipInstallSpec FromResource(const std::string& resourcePath);

    /** \brief Save this PipInstallSpec to a JSON file.
     *
     * \param[in] path File system path to write.
     * \throws std::runtime_error If the file cannot be written.
     */
    void SaveToFile(const std::string& path) const;
  };

  MITKPYTHONINSTALLER_EXPORT void from_json(const nlohmann::ordered_json& j, PipInstallGroup& g);
  MITKPYTHONINSTALLER_EXPORT void to_json(nlohmann::ordered_json& j, const PipInstallGroup& g);

  MITKPYTHONINSTALLER_EXPORT void from_json(const nlohmann::ordered_json& j, HuggingFaceDownload& d);
  MITKPYTHONINSTALLER_EXPORT void to_json(nlohmann::ordered_json& j, const HuggingFaceDownload& d);

  MITKPYTHONINSTALLER_EXPORT void from_json(const nlohmann::ordered_json& j, PipInstallSpec& s);
  MITKPYTHONINSTALLER_EXPORT void to_json(nlohmann::ordered_json& j, const PipInstallSpec& s);
}

#endif
