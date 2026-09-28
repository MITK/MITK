/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVolumeBlendMode_h
#define mitkVolumeBlendMode_h

#include <MitkVolumeVisualizationExports.h>

#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  class DataNode;

  /**
   * \brief The rule by which the samples along one viewing ray are combined
   *        into one pixel.
   *
   * A subset of VTK's blend modes: the five that reinterpret the transfer
   * function already on the node and need nothing else. Isosurface and slice
   * are deliberately absent, because each needs input of its own that nothing
   * in MITK supplies - iso-values, or a plane. The ray caster accepts both
   * regardless and then draws nothing, so offering either would buy an empty
   * 3D window rather than a degraded image.
   *
   * Which one applies is part of a transfer function's recipe rather than an
   * independent axis: the projection modes reduce each ray to a single scalar
   * and then run it through the transfer function, so a curve authored to
   * classify tissue and one authored as a window are not interchangeable
   * between them. Presets therefore name the mode they were authored for.
   */
  enum class VolumeBlendMode
  {
    Composite,
    MaximumIntensity,
    MinimumIntensity,
    AverageIntensity,
    Additive
  };

  /**
   * \brief Key under which SetVolumeBlendMode records the mode.
   *
   * Read by mitk::VolumeMapperVtkSmart3D, as a plain VTK enum value, which is
   * why the stored representation is ToVtkBlendMode's rather than this enum's
   * own ordering.
   */
  constexpr const char *VOLUME_BLEND_MODE_PROPERTY = "volumerendering.blendmode";

  /** \brief One blend mode as it is named in files and presented in controls. */
  struct MITKVOLUMEVISUALIZATION_EXPORT VolumeBlendModeDescription
  {
    VolumeBlendMode mode;

    /**
     * \brief Stable identifier, used in the preset file format.
     *
     * Kept apart from the label so that renaming a control cannot change what
     * an already-saved transfer function means.
     */
    std::string id;

    /** \brief Name for a control that selects the mode. */
    std::string label;

    /** \brief One sentence on what the mode shows, short enough for a tooltip. */
    std::string description;

    /** \brief The modes on offer, in the order they should be presented. */
    static const std::vector<VolumeBlendModeDescription> &GetAll();

    static const VolumeBlendModeDescription *FromMode(VolumeBlendMode mode);

    /**
     * \brief The mode with the given identifier.
     * \return The description, or nullptr for an identifier no mode uses.
     */
    static const VolumeBlendModeDescription *FromId(const std::string &id);
  };

  /** \brief The vtkVolumeMapper::BlendModes value for \p mode. */
  MITKVOLUMEVISUALIZATION_EXPORT int ToVtkBlendMode(VolumeBlendMode mode);

  /**
   * \brief Record the blend mode on the node, for the mapper to pick up.
   * \param[in] node The node to configure.
   * \throws mitk::Exception if \p node is nullptr.
   */
  MITKVOLUMEVISUALIZATION_EXPORT void SetVolumeBlendMode(DataNode *node, VolumeBlendMode mode);

  /**
   * \brief The mode the node renders in.
   * \param[in] node The node to inspect; nullptr is allowed.
   * \return Composite for a node that names no mode: that is the fallback the
   *         mapper applies, so it is what such a node actually renders as.
   *         Nothing for a null node, and for a mode outside the set MITK
   *         offers - the property is a plain int, so the Properties view and
   *         any scene file can put one there, and reporting it as composite
   *         would both mislabel the node and promise lighting the ray caster
   *         will never apply. A caller that has to show something either way
   *         wants value_or(VolumeBlendMode::Composite), which is once more
   *         what the mapper does.
   */
  MITKVOLUMEVISUALIZATION_EXPORT std::optional<VolumeBlendMode> GetVolumeBlendMode(const DataNode *node);
}

#endif
