/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureLegacyProperties_h
#define mitkPlanarFigureLegacyProperties_h

namespace mitk::PlanarFigureLegacyProperties
{
  /**
   * \brief Registers with the property migration service how properties of
   *        planar figures in scenes of previous versions are migrated.
   *
   * Line width and font size move to the keys the 2D mapper reads now. The
   * other render and interaction properties of the previous mapper and
   * interactor, and the state the previous figures kept in their data
   * property lists, are removed.
   */
  void Register();
}

#endif
