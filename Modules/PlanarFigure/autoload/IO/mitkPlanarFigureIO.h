/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureIO_h
#define mitkPlanarFigureIO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  /**
   * \brief Reads and writes a planar figure in JSON ("*.pf.json").
   *
   * \code{.json}
   * {
   *   "FileFormat": "MITK Planar Figure",
   *   "Version": 1,
   *   "Type": "double_ellipse",
   *   "Plane": { "Transform": [...], "Bounds": [0, 256, 0, 256, 0, 1], "ImageGeometry": true },
   *   "ControlPoints": [[81.56, 175.64], [104.17, 137.69], [65.97, 166.35]],
   *   "Parameters": { "Thickness": 12.3 },
   *   "ShapeLocked": true
   * }
   * \endcode
   *
   * "Type" is the type id of PlanarFigure::GetTypeId(). Control points are in
   * plane coordinates (mm). "Parameters" holds the thickness of a double
   * ellipse and the reference direction of an ellipse, if it has one;
   * "ShapeLocked" is only written if true. Reading throws mitk::Exception on
   * other formats, newer versions, unknown types and invalid data, and
   * ignores unknown top-level keys.
   */
  class PlanarFigureIO : public AbstractFileIO
  {
  public:
    PlanarFigureIO();

    using AbstractFileReader::Read;

    /** \brief Writes a complete planar figure; throws mitk::Exception otherwise. */
    void Write() override;

    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    PlanarFigureIO *IOClone() const override;
  };
}

#endif
