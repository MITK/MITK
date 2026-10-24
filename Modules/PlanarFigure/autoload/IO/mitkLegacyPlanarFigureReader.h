/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLegacyPlanarFigureReader_h
#define mitkLegacyPlanarFigureReader_h

#include <mitkAbstractFileReader.h>

namespace mitk
{
  /**
   * \brief Reads planar figures from the former XML format ("*.pf").
   *
   * Figures of that format are converted to their current representation:
   * polygons that are not closed become paths, crosses with a single line
   * become lines, rectangles keep two opposite corners, and the fourth point
   * of ellipses and double ellipses becomes their reference direction and
   * thickness. Circles and double ellipses of fixed size, whose size was not
   * stored, are read with a radius of 0. Properties stored in the file only
   * steer the conversion.
   *
   * Reading throws mitk::Exception on versions other than 1, unknown types,
   * missing or malformed geometry, control points that are not numbered
   * from 0, and figures that violate the constraints of their type.
   */
  class LegacyPlanarFigureReader : public AbstractFileReader
  {
  public:
    LegacyPlanarFigureReader();

    using AbstractFileReader::Read;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    LegacyPlanarFigureReader *Clone() const override;
  };
}

#endif
