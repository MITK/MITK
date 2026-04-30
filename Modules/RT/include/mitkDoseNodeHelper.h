/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkDoseNodeHelper_h
#define mitkDoseNodeHelper_h

#include <mitkIsoDoseLevelCollections.h>
#include <mitkDataNode.h>
#include <MitkRTExports.h>

namespace mitk
{
  /**
   * \brief Configures a DataNode so it is correctly visualized as a dose distribution with color wash.
   *
   * Sets all necessary properties on the given node for dose visualization including
   * the iso dose level set, reference dose, color wash visibility, rendering mode,
   * transfer function, and opacity.
   *
   * \param[in,out] doseNode The DataNode containing the dose image to configure. If nullptr, the function returns immediately.
   * \param[in] colorPreset The iso dose level set defining the color coding for visualization.
   * \param[in] referenceDose The absolute reference dose (in Gy) used to scale relative iso dose levels.
   * \param[in] showColorWashGlobal If true, a color transfer function is generated and applied;
   *            otherwise, lookup-table level-window mode is used.
   *
   * \pre The DataNode must contain a valid mitk::Image as its data.
   *
   * \sa ConfigureNodeAsIsoLineNode
   * \sa mitk::IsoDoseLevelSet
   * \sa mitk::DoseImageVtkMapper2D
   */
  void MITKRT_EXPORT ConfigureNodeAsDoseNode(mitk::DataNode* doseNode,
    const mitk::IsoDoseLevelSet* colorPreset,
    mitk::DoseValueAbs referenceDose,
    bool showColorWashGlobal = true);

  /**
   * \brief Configures a DataNode so it is correctly visualized as dose iso line contours.
   *
   * Sets all necessary properties on the given node for iso line rendering including
   * the iso dose level set, free iso values, reference dose, iso line visibility,
   * outline rendering, and assigns a DoseImageVtkMapper2D for contour drawing.
   *
   * \param[in,out] doseOutlineNode The DataNode containing the dose image for iso line rendering. If nullptr, no action is taken.
   * \param[in] colorPreset The iso dose level set defining the colors and visibility of each iso line.
   * \param[in] referenceDose The absolute reference dose (in Gy) used to scale relative iso dose levels.
   * \param[in] showIsolinesGlobal If true, iso line rendering is globally enabled on the node.
   *
   * \pre The DataNode must contain a valid mitk::Image as its data.
   *
   * \sa ConfigureNodeAsDoseNode
   * \sa mitk::DoseImageVtkMapper2D
   */
  void MITKRT_EXPORT ConfigureNodeAsIsoLineNode(mitk::DataNode* doseOutlineNode,
    const mitk::IsoDoseLevelSet* colorPreset,
    mitk::DoseValueAbs referenceDose,
    bool showIsolinesGlobal = true);


}

#endif
