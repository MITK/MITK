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

#include <MitkExports.h>
#include <itkObject.h>
#include <Overlays/mitkBaseLayouter.h>

namespace mitk {

/** @brief A simple implementation of a layouter for 2D Overlays
 *
 *  @ingroup Overlays
 */
class MITK_CORE_EXPORT Overlay2DLayouter : public mitk::BaseLayouter {
public:
  enum Alignment{TopLeft, Top, TopRight, BottomLeft, Bottom, BottomRight};
  mitkClassMacro(Overlay2DLayouter, mitk::BaseLayouter);
  itkNewMacro(Overlay2DLayouter);
  static const char* STANDARD_2D_TOPLEFT;
  static const char* STANDARD_2D_TOP;
  static const char* STANDARD_2D_TOPRIGHT;
  static const char* STANDARD_2D_BOTTOMLEFT;
  static const char* STANDARD_2D_BOTTOM;
  static const char* STANDARD_2D_BOTTOMRIGHT;

  /** \brief Factory method for the different kinds of Layouters */
  /** Create a Layouter that, depending on the identifier sorts the Overlays in one corner of the Renderwindow*/
  static Overlay2DLayouter::Pointer CreateLayouter(Alignment alignment, BaseRenderer* renderer)
  {
    if(renderer == NULL)
      return NULL;
    std::string identifier;
    switch (alignment) {
    case TopLeft:
      identifier = STANDARD_2D_TOPLEFT;
      break;
    case Top:
      identifier = STANDARD_2D_TOP;
      break;
    case TopRight:
      identifier = STANDARD_2D_TOPRIGHT;
      break;
    case BottomLeft:
      identifier = STANDARD_2D_BOTTOMLEFT;
      break;
    case Bottom:
      identifier = STANDARD_2D_BOTTOM;
      break;
    case BottomRight:
      identifier = STANDARD_2D_BOTTOMRIGHT;
      break;
    default:
      return NULL;
    }

    mitk::Overlay2DLayouter::Pointer layouter = mitk::Overlay2DLayouter::New();
    layouter->m_Alignment = alignment;
    layouter->m_Identifier = identifier;
    layouter->SetBaseRenderer(renderer);
    return layouter;

  }

  /** \brief Factory method for the different kinds of Layouters */
  /** Create a Layouter that, depending on the identifier sorts the Overlays in one corner of the Renderwindow*/
  static Overlay2DLayouter::Pointer CreateLayouter(std::string identifier, BaseRenderer* renderer)
  {
    if(renderer == NULL)
      return NULL;
    Alignment alignment;
    if(identifier.compare(0,11,"STANDARD_2D") != 0)
      return NULL;
    if(identifier.compare(STANDARD_2D_TOPLEFT) == 0)
      alignment = TopLeft;
    else if(identifier.compare(STANDARD_2D_TOP) == 0)
      alignment = Top;
    else if(identifier.compare(STANDARD_2D_TOPRIGHT) == 0)
      alignment = TopRight;
    else if(identifier.compare(STANDARD_2D_BOTTOMLEFT) == 0)
      alignment = BottomLeft;
    else if(identifier.compare(STANDARD_2D_BOTTOM) == 0)
      alignment = Bottom;
    else if(identifier.compare(STANDARD_2D_BOTTOMRIGHT) == 0)
      alignment = BottomRight;
    else return NULL;

    mitk::Overlay2DLayouter::Pointer layouter = mitk::Overlay2DLayouter::New();
    layouter->m_Alignment = alignment;
    layouter->m_Identifier = identifier;
    layouter->SetBaseRenderer(renderer);
    return layouter;
  }

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


