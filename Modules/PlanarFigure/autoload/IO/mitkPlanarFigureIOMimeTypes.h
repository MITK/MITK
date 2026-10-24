/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureIOMimeTypes_h
#define mitkPlanarFigureIOMimeTypes_h

#include <mitkCustomMimeType.h>

namespace mitk::PlanarFigureIOMimeTypes
{
  /** \brief The value of "FileFormat" in planar figure files. */
  constexpr const char *FileFormat = "MITK Planar Figure";

  /**
   * \brief Planar figures in JSON ("*.pf.json").
   *
   * An existing file named "*.json" applies by its content, so planar
   * figures in such files are recognized as well.
   */
  class PlanarFigureMimeType : public CustomMimeType
  {
  public:
    PlanarFigureMimeType();

    bool AppliesTo(const std::string &path) const override;
    PlanarFigureMimeType *Clone() const override;
  };

  PlanarFigureMimeType PLANARFIGURE_MIMETYPE();
  std::string PLANARFIGURE_MIMETYPE_NAME();

  /** \brief Planar figures in the former XML format ("*.pf"), which is only read. */
  CustomMimeType LEGACY_PLANARFIGURE_MIMETYPE();
  std::string LEGACY_PLANARFIGURE_MIMETYPE_NAME();
}

#endif
