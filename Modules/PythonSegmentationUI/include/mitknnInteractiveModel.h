/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveModel_h
#define mitknnInteractiveModel_h

#include <MitkPythonSegmentationUIExports.h>

#include <string>
#include <vector>

namespace mitk
{
  namespace nnInteractive
  {
    /** \brief A model checkpoint entry as reported by nnInteractive's model
     *         management.
     *
     * \sa ListModels()
     */
    struct ModelInfo
    {
      std::string Id;          /**< \brief Model id, e.g. "nnInteractive_v1.0". */
      std::string DisplayName; /**< \brief Human-readable name (falls back to Id). */
      bool IsDefault = false;  /**< \brief Whether this is the manifest's recommended default. */
      bool Downloaded = false; /**< \brief Whether the checkpoint is already available locally. */
    };

    /** \brief Outcome of comparing the selected checkpoint against the library's
     *         recommended default.
     *
     * \sa CheckModelUpdate()
     */
    enum class ModelUpdateStatus
    {
      Unknown,        /**< \brief Could not determine (offline, no model management, or empty list). */
      UpToDate,       /**< \brief The selection already matches the recommended default. */
      UpdateAvailable /**< \brief A different checkpoint is now recommended as the default. */
    };

    /** \brief Result of CheckModelUpdate(). */
    struct ModelCheckResult
    {
      ModelUpdateStatus Status = ModelUpdateStatus::Unknown;
      std::string CurrentId;     /**< \brief The model id that was checked. */
      std::string RecommendedId; /**< \brief The manifest's default model id (empty when unknown). */
    };

    /** \brief Lists the model checkpoints known to nnInteractive's model management.
     *
     * Runs \c nnInteractive.model_management.list_models() in a short-lived
     * subprocess driven by the virtual environment's own Python interpreter,
     * rather than the embedded interpreter. This keeps model management (and its
     * transitive native dependencies, e.g. PyYAML) out of the host process, so it
     * never maps libraries from the virtual environment that would block a later
     * in-place update on Windows. The call refreshes the model manifest from
     * Hugging Face (remote-first, with an offline cache fallback).
     *
     * Returns an empty list when the virtual environment or its interpreter is
     * missing, model management is unavailable (a client-only install), or the
     * query fails, so callers can treat "no models" as "fall back to free-text
     * entry".
     *
     * \param[in] venvName The nnInteractive virtual environment to query.
     *
     * \return The known model checkpoints (possibly empty).
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT std::vector<ModelInfo> ListModels(const std::string& venvName);

    /** \brief Checks whether a checkpoint newer than the selected one is recommended.
     *
     * Compares \p selectedModelId against \c get_default_model_id() (queried in a
     * subprocess, see ListModels()): the result is UpdateAvailable when the default
     * is non-empty and differs from the selection, UpToDate when they match (or when
     * the selection is empty, i.e. already tracking the default), and Unknown on any
     * failure (offline, no model management). It deliberately does not parse version
     * suffixes; "newer" means "the library now recommends a different default
     * checkpoint".
     *
     * \param[in] venvName The nnInteractive virtual environment to query.
     * \param[in] selectedModelId The model id currently configured (may be empty to
     *                    mean "use the recommended default").
     *
     * \return A ModelCheckResult; Status is Unknown when it could not be determined
     *         (the caller should then not prompt).
     *
     * \sa ModelUpdateStatus
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT ModelCheckResult CheckModelUpdate(const std::string& venvName, const std::string& selectedModelId);
  }
}

#endif
