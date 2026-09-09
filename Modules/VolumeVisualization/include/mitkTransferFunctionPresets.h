/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTransferFunctionPresets_h
#define mitkTransferFunctionPresets_h

#include <MitkVolumeVisualizationExports.h>

#include <mitkTransferFunction.h>
#include <mitkVolumeBlendMode.h>

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
   * and RGBPoints arrays plus a ColorSpace, extended by a BlendMode.
   *
   * The blend mode is part of a preset rather than an independent setting
   * because the projection modes reduce each ray to one scalar before running
   * it through the transfer function: a curve authored to classify tissue and
   * one authored as a window are not interchangeable between them. A file that
   * names no BlendMode - a colormap taken from elsewhere - is read as
   * composite, which is what every curve authored without the question in mind
   * assumes.
   *
   * The same format is used to save and load individual user-created transfer
   * functions (see SaveTransferFunction / LoadTransferFunction), so a saved
   * file is structurally identical to one MedicalColorPresets.json entry.
   */
  class MITKVOLUMEVISUALIZATION_EXPORT TransferFunctionPresets
  {
  public:
    TransferFunctionPresets();

    /** \brief Names of the available presets, in file order. */
    std::vector<std::string> GetPresetNames() const;

    /**
     * \brief Build a transfer function for the named preset, and report the
     *        blend mode it was authored for.
     * \param[in] presetName One of the names returned by GetPresetNames().
     * \param[out] blendMode The mode to render the returned curve in. Required
     *             rather than defaulted, for the same reason
     *             SaveTransferFunction requires it: the two only mean anything
     *             together, and a projection curve rendered as composite is
     *             silently wrong rather than visibly so. Left untouched when
     *             the name is unknown.
     * \return A newly created transfer function, or nullptr if the name is unknown.
     */
    mitk::TransferFunction::Pointer CreateTransferFunction(const std::string &presetName,
      VolumeBlendMode &blendMode) const;

    /**
     * \brief Load a transfer function stored in the preset JSON format from
     * any stream (a file, the embedded resource, an in-memory buffer).
     * \param[in] stream The input stream to read from.
     * \param[out] blendMode The mode the loaded function was authored for;
     *             composite for a file that names none. Required for the same
     *             reason as in CreateTransferFunction. Left untouched when the
     *             stream holds no valid entry.
     * \return The transfer function, or nullptr if the stream holds no valid
     * preset entry.
     */
    static mitk::TransferFunction::Pointer LoadTransferFunction(std::istream &stream,
      VolumeBlendMode &blendMode);

    /**
     * \brief Write a transfer function to a stream in the preset JSON format:
     * a one-element array identical to a MedicalColorPresets.json entry
     * (Name, ColorSpace, BlendMode, OpacityPoints, RGBPoints, EffectiveRange).
     * The gradient opacity component is not stored.
     * \param[in] stream The output stream to write to.
     * \param[in] name The preset name to store.
     * \param[in] transferFunction The transfer function to serialize.
     * \param[in] blendMode The mode the function is meant to be rendered with.
     *            Required rather than defaulted: a curve saved out of a
     *            projection mode and read back as composite is silently wrong,
     *            and the caller is the only one who knows which it was.
     * \return True on success.
     */
    static bool SaveTransferFunction(std::ostream &stream, const std::string &name,
      mitk::TransferFunction *transferFunction, VolumeBlendMode blendMode);

  private:
    struct Preset
    {
      std::string name;
      std::string colorSpace;
      VolumeBlendMode blendMode {VolumeBlendMode::Composite};
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
