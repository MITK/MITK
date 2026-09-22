/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVolumeRenderingMaterial_h
#define mitkVolumeRenderingMaterial_h

#include <MitkVolumeVisualizationExports.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief The shading and Phong material properties of a volume-rendering
   *        node, as one value.
   *
   * The field initialisers are the one definition of MITK's volume material
   * defaults. mitk::VolumeMapperVtkSmart3D registers them from here, and FromNode
   * falls back to them for a node carrying none of the properties - a lasting
   * state, since SetDefaultProperties runs only from DataNode::SetData, only via
   * IOExt's mapper provider, and only for an already initialized image.
   *
   * They describe a node nobody configured, which renders under the renderer's
   * default five-light kit. The "studio" mitk::VolumeRenderingLightingModel
   * repeats them, so that state can be chosen deliberately rather than only
   * fallen into; the two have to stay in step. The tuned models hold different
   * values because ambient cannot cross between them - the shader computes it
   * differently depending on how many lights are switched on.
   */
  struct MITKVOLUMEVISUALIZATION_EXPORT VolumeRenderingMaterial
  {
    /**
     * \brief Whether the ray caster shades at all.
     *
     * With this off VTK ignores the four values below, and the scattering
     * properties a lighting model writes alongside them.
     */
    bool shade = true;

    float ambient = 0.1f;
    float diffuse = 0.50f;
    float specular = 0.40f;
    float specularPower = 16.0f;

    /**
     * \brief Read a node's material properties.
     * \param[in] node The node to read; nullptr yields the defaults.
     * \return The values found, with anything the node does not carry left at
     *         its default. Absence is not reported separately because a caller
     *         driving controls needs a value for each regardless - there is no
     *         slider position that means "unset".
     */
    static VolumeRenderingMaterial FromNode(const DataNode *node);

    /**
     * \brief Write all five properties onto the node.
     * \param[in] node The node to configure.
     * \throws mitk::Exception if \p node is nullptr.
     */
    void ApplyTo(DataNode *node) const;

    /**
     * \brief Keys under which the five properties are stored.
     *
     * Public because mitk::VolumeMapperVtkSmart3D both registers the defaults
     * under them and reads them back each frame. Two spellings of one name fail
     * silently, leaving a slider that moves a value nothing renders.
     */
    static constexpr const char *SHADE_PROPERTY = "volumerendering.shade";
    static constexpr const char *AMBIENT_PROPERTY = "volumerendering.ambient";
    static constexpr const char *DIFFUSE_PROPERTY = "volumerendering.diffuse";
    static constexpr const char *SPECULAR_PROPERTY = "volumerendering.specular";
    static constexpr const char *SPECULAR_POWER_PROPERTY = "volumerendering.specular.power";
  };
}

#endif
