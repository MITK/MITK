/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKOVERLAY2DLAYOUTER_H
#define MITKOVERLAY2DLAYOUTER_H

#include <MitkOverlaysExports.h>
#include <mitkAbstractOverlayLayouter.h>

namespace mitk {

/** @brief A simple implementation of a layouter for 2D Overlays
 */
class MitkOverlays_EXPORT Overlay2DLayouter : public mitk::AbstractOverlayLayouter {
public:
  enum Alignment{TopLeft, Top, TopRight, BottomLeft, Bottom, BottomRight};
  mitkClassMacro(Overlay2DLayouter, mitk::AbstractOverlayLayouter);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  static std::string STANDARD_2D_TOPLEFT();
  static std::string STANDARD_2D_TOP();
  static std::string STANDARD_2D_TOPRIGHT();
  static std::string STANDARD_2D_BOTTOMLEFT();
  static std::string STANDARD_2D_BOTTOM();
  static std::string STANDARD_2D_BOTTOMRIGHT();

  /** \brief Factory method for the different kinds of Layouters */
  /** Create a Layouter that, depending on the identifier sorts the Overlays in one corner of the Renderwindow*/
  static Overlay2DLayouter::Pointer CreateLayouter(Alignment alignment, BaseRenderer* renderer);

  /** \brief Factory method for the different kinds of Layouters */
  /** Create a Layouter that, depending on the identifier sorts the Overlays in one corner of the Renderwindow*/
  static Overlay2DLayouter::Pointer CreateLayouter(const std::string& identifier, BaseRenderer* renderer);

  void PrepareLayout();
protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit Overlay2DLayouter();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~Overlay2DLayouter();

  Alignment m_Alignment;

  double m_Margin;

private:

  /** \brief copy constructor */
  Overlay2DLayouter( const Overlay2DLayouter &);

  /** \brief assignment operator */
  Overlay2DLayouter &operator=(const Overlay2DLayouter &);
};

} // namespace mitk
#endif // MITKOVERLAY2DLAYOUTER_H


