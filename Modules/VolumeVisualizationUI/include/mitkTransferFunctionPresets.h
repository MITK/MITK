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

#include <iosfwd>
#include <string>
#include <vector>
#include <array>

namespace mitk
{
  /**
   * \brief Catalog of built-in volume-rendering transfer function presets.
   *
   * Parses the embedded module resource MedicalColorPresets.json once on
   * construction and builds mitk::TransferFunction instances on demand. The
   * presets use the ParaView / 3D-Slicer colormap format: flat OpacityPoints
   * and RGBPoints arrays plus a ColorSpace.
   *
   * The same format is used to save and load individual user-created transfer
   * functions (see SaveTransferFunction / LoadTransferFunction), so a saved
   * file is structurally identical to one MedicalColorPresets.json entry.
   */
  class MITKVOLUMEVISUALIZATIONUI_EXPORT TransferFunctionPresets
  {
  public:
    TransferFunctionPresets();

    /** \brief Names of the available presets, in file order. */
    std::vector<std::string> GetPresetNames() const;

    /**
     * \brief Intensity window the preset is authored for.
     * \return {min, max} scalar values in the image's own intensity units
     * (e.g. Hounsfield units for CT) over which the preset's transfer
     * function is meaningful. Callers use it to window/scale the volume
     * so the preset lands on the right value range.
     */
    std::array<double, 2> GetEffectiveRange(const std::string &presetName) const;

    /**
     * \brief Build a transfer function for the named preset.
     * \param[in] presetName One of the names returned by GetPresetNames().
     * \return A newly created transfer function, or nullptr if the name is unknown.
     */
    mitk::TransferFunction::Pointer CreateTransferFunction(const std::string &presetName) const;

    /**
     * \brief Load a transfer function stored in the preset JSON format from
     * any stream (a file, the embedded resource, an in-memory buffer).
     * \param[in] stream The input stream to read from.
     * \return The transfer function, or nullptr if the stream holds no valid
     * preset entry.
     */
    static mitk::TransferFunction::Pointer LoadTransferFunction(std::istream &stream);

    /**
     * \brief Write a transfer function to a stream in the preset JSON format:
     * a one-element array identical to a MedicalColorPresets.json entry
     * (Name, ColorSpace, OpacityPoints, RGBPoints, EffectiveRange). The
     * gradient opacity component is not stored.
     * \param[in] stream The output stream to write to.
     * \param[in] name The preset name to store.
     * \param[in] transferFunction The transfer function to serialize.
     * \return True on success.
     */
    static bool SaveTransferFunction(std::ostream &stream, const std::string &name,
      mitk::TransferFunction *transferFunction);

  private:
    struct Preset
    {
      std::string name;
      std::string colorSpace;
      TransferFunction::ControlPoints scalarOpacity;
      TransferFunction::RGBControlPoints color;
      std::array<double, 2> effectiveRange {0.0, 0.0};
    };

    /**
     * \brief Parse presets from a stream in the MedicalColorPresets.json
     * format. Accepts either a catalog array or a single entry object.
     */
    static std::vector<Preset> ReadPresets(std::istream &stream);

    /** \brief Build a transfer function from a decoded preset. */
    static mitk::TransferFunction::Pointer BuildTransferFunction(const Preset &preset);

    std::vector<Preset> m_Presets;
  };
}

#endif
