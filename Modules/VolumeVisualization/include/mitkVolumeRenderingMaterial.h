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
   * The defaults mirror what mitk::VolumeMapperVtkSmart3D registers in
   * SetDefaultProperties. They are repeated here because that only runs via the
   * IOExt object factory, which nothing guarantees, so a node can legitimately
   * carry none of these.
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
  };
}

#endif
