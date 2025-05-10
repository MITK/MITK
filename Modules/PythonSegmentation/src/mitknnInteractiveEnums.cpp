/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveEnums.h>

const std::string& mitk::nnInteractive::GetInteractionTypeAsString(InteractionType interactionType)
{
  static const std::array<std::string, 4> strings = {
    "Point",
    "Box",
    "Scribble",
    "Lasso"
  };

  return strings.at(static_cast<size_t>(interactionType));
}

const std::array<mitk::nnInteractive::InteractionType, 4>& mitk::nnInteractive::GetAllInteractionTypes()
{
  static constexpr std::array<InteractionType, 4> interactionTypes = {
    InteractionType::Point,
    InteractionType::Box,
    InteractionType::Scribble,
    InteractionType::Lasso
  };

  return interactionTypes;
}

const std::string& mitk::nnInteractive::GetPromptTypeAsString(PromptType promptType)
{
  static const std::array<std::string, 2> strings = {
    "Positive",
    "Negative"
  };

  return strings.at(static_cast<size_t>(promptType));
}

const std::array<mitk::nnInteractive::PromptType, 2>& mitk::nnInteractive::GetAllPromptTypes()
{
  static constexpr std::array<PromptType, 2> promptTypes = {
    PromptType::Positive,
    PromptType::Negative
  };

  return promptTypes;
}

const std::string& mitk::nnInteractive::GetBackendAsString(Backend backend)
{
  static const std::array<std::string, 2> strings = {
    "CUDA",
    "CPU"
  };

  return strings.at(static_cast<size_t>(backend));
}

const std::array<mitk::nnInteractive::Backend, 2>& mitk::nnInteractive::GetAllBackends()
{
  static constexpr std::array<Backend, 2> backends = {
    Backend::CUDA,
    Backend::CPU
  };

  return backends;
}

const mitk::Color& mitk::nnInteractive::GetColor(PromptType promptType, ColorIntensity colorIntensity)
{
  static const auto vibrantPositiveColor = MakeColor(0.0f, 0.694f, 0.047f); // #00b10c
  static const auto mutedPositiveColor = MakeColor(0.0f, 0.472f, 0.027f);   // #007807
  static const auto vibrantNegativeColor = MakeColor(0.827f, 0.027f, 0.0f); // #d30700
  static const auto mutedNegativeColor = MakeColor(0.561f, 0.02f, 0.0f);    // #8f0500

  if (promptType == PromptType::Positive)
  {
    return colorIntensity == ColorIntensity::Vibrant
      ? vibrantPositiveColor
      : mutedPositiveColor;
  }

  return colorIntensity == ColorIntensity::Vibrant
    ? vibrantNegativeColor
    : mutedNegativeColor;
}
