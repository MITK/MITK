/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveEnums_h
#define mitknnInteractiveEnums_h

#include <mitkColorProperty.h>
#include <MitkPythonSegmentationExports.h>

namespace mitk::nnInteractive
{
  /** \brief Specifies the types of interactions available.
   *
   * Interaction types represent the different modes of interaction for
   * prompting available in %nnInteractive, each corresponding to a specific
   * user action.
   *
   * \note Whenever modifying this enum class, make sure to also adapt its
   *       utility functions GetInteractionTypeAsString() and
   *       GetAllInteractionTypes().
   *
   * \see Interactor::Interactor(), GetInteractionTypeAsString(),
   *      GetAllInteractionTypes()
   */
  enum class InteractionType
  {
    Point,    /**< Interaction for placing individual points */
    Box,      /**< Interaction for drawing rectangular boxes */
    Scribble, /**< Interaction for freehand brushstrokes */
    Lasso     /**< Interaction for drawing contours */
  };

  /** \brief Converts an InteractionType to a corresponding string representation.
   */
  MITKPYTHONSEGMENTATION_EXPORT const std::string& GetInteractionTypeAsString(InteractionType interactionType);

  /** \brief Returns all possible interaction types.
   */
  MITKPYTHONSEGMENTATION_EXPORT const std::array<InteractionType, 4>& GetAllInteractionTypes();

  /** \brief Specifies the interaction modes for interactors.
   *
   * Interaction modes define specific behaviors during interactions, such as
   * blocking left mouse button display interactions resp. crosshair navigation.
   */
  enum class InteractionMode
  {
    Default,                   /**< Default interaction mode */
    BlockLMBDisplayInteraction /**< Block left mouse button display interaction */
  };

  /** \brief Specifies the types of prompts used in %nnInteractive.
   *
   * Prompt types distinguish between positive and negative prompts, e.g.,
   * should the labeled region by an interaction be included in or excluded from
   * the resulting segmentation.
   *
   * \note Whenever modifying this enum class, make sure to also adapt its
   *       utility functions GetPromptTypeAsString() and GetAllPromptTypes().
   *
   * \see GetPromptTypeAsString(), GetAllPromptTypes()
   */
  enum class PromptType
  {
    Positive, /**< Include region in segmentation */
    Negative  /**< Exclude region from segmentation */
  };

  /** \brief Converts a PromptType to a corresponding string representation.
   */
  MITKPYTHONSEGMENTATION_EXPORT const std::string& GetPromptTypeAsString(PromptType promptType);

  /** \brief Returns all possible prompt types.
   */
  MITKPYTHONSEGMENTATION_EXPORT const std::array<PromptType, 2>& GetAllPromptTypes();

  /** \brief Specifies the computation backends available for %nnInteractive.
   *
   * Backends define the computational resources used, such as GPU (CUDA) or
   * CPU for processing.
   *
   * \note Whenever modifying this enum class, make sure to also adapt its
   *       utility functions GetBackendAsString() and GetAllBackends().
   *
   * \see GetBackendAsString(), GetAllBackends()
   */
  enum class Backend
  {
    CUDA, /**< CUDA backend for GPU computation (fast) */
    CPU   /**< Backend for CPU computation (slow) */
  };

  /** \brief Converts a Backend type to a corresponding string representation.
   */
  MITKPYTHONSEGMENTATION_EXPORT const std::string& GetBackendAsString(Backend backend);

  /** \brief Returns all possible backend types.
   */
  MITKPYTHONSEGMENTATION_EXPORT const std::array<Backend, 2>& GetAllBackends();

  /** \brief Specifies the intensity of the colors used in an interaction.
   *
   * \see GetColor()
   */
  enum class ColorIntensity
  {
    Muted,  /**< Muted color intensity */
    Vibrant /**< Vibrant color intensity */
  };

  /** \brief Returns the color associated with a specific prompt type and color intensity.
   *
   * Use this function for color lookup to ensure a coherent color scheme across
   * different interactors.
   */
  MITKPYTHONSEGMENTATION_EXPORT const Color& GetColor(PromptType promptType, ColorIntensity colorIntensity);
}

#endif
