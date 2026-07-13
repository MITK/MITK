/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionPresets_h
#define mitkTransferFunctionPresets_h

#include <MitkVolumeVisualizationUIExports.h>

#include <mitkTransferFunction.h>

#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Catalog of built-in volume-rendering transfer function presets.
   *
   * Parses the embedded module resource MedicalColorPresets.json once on
   * construction and builds mitk::TransferFunction instances on demand. The
   * presets use the ParaView / 3D-Slicer colormap format: flat OpacityPoints
   * and RGBPoints arrays plus a ColorSpace.
   */
  class MITKVOLUMEVISUALIZATIONUI_EXPORT TransferFunctionPresets
  {
  public:
    TransferFunctionPresets();

    /** \brief Names of the available presets, in file order. */
    std::vector<std::string> GetPresetNames() const;

    /**
     * \brief Build a transfer function for the named preset.
     * \param[in] presetName One of the names returned by GetPresetNames().
     * \return A newly created transfer function, or nullptr if the name is unknown.
     */
    mitk::TransferFunction::Pointer CreateTransferFunction(const std::string &presetName) const;

  private:
    struct Preset
    {
      std::string name;
      std::string colorSpace;
      TransferFunction::ControlPoints scalarOpacity;
      TransferFunction::RGBControlPoints color;
    };

    std::vector<Preset> m_Presets;
  };
}

#endif
