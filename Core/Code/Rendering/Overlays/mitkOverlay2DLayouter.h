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
#include "mitkBaseLayouter.h"

namespace mitk {

class MITK_CORE_EXPORT Overlay2DLayouter : public mitk::BaseLayouter {
public:
  enum Alignment{TopLeft, Top, TopRight, Left, Middle, Right, BottomLeft, Bottom, BottomRight};
  mitkClassMacro(Overlay2DLayouter, mitk::BaseLayouter);
  itkNewMacro(Overlay2DLayouter);
  static const std::string STANDARD_2D_TOPLEFT;
  static const std::string STANDARD_2D_TOP;
  static const std::string STANDARD_2D_TOPRIGHT;
  static const std::string STANDARD_2D_LEFT;
  static const std::string STANDARD_2D_MIDDLE;
  static const std::string STANDARD_2D_RIGHT;
  static const std::string STANDARD_2D_BOTTOMLEFT;
  static const std::string STANDARD_2D_BOTTOM;
  static const std::string STANDARD_2D_BOTTOMRIGHT;

  static Overlay2DLayouter::Pointer CreateLayouter(Alignment alignment)
  {
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
    case Left:
      identifier = STANDARD_2D_LEFT;
      break;
    case Middle:
      identifier = STANDARD_2D_MIDDLE;
      break;
    case Right:
      identifier = STANDARD_2D_RIGHT;
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
    return layouter;

  }

  static Overlay2DLayouter::Pointer CreateLayouter(std::string identifier)
  {
    Alignment alignment;
    if(identifier.compare(0,11,"STANDARD_2D") != 0)
      return NULL;
    if(identifier.compare(12,identifier.size()-1,STANDARD_2D_TOPLEFT) == 0)
      alignment = TopLeft;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_TOP) == 0)
      alignment = Top;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_TOPRIGHT) == 0)
      alignment = TopRight;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_LEFT) == 0)
      alignment = Left;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_MIDDLE) == 0)
      alignment = Middle;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_RIGHT) == 0)
      alignment = Right;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_BOTTOMLEFT) == 0)
      alignment = BottomLeft;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_BOTTOM) == 0)
      alignment = Bottom;
    else if(identifier.compare(12,identifier.size()-1,STANDARD_2D_BOTTOMRIGHT) == 0)
      alignment = BottomRight;
    else return NULL;

    mitk::Overlay2DLayouter::Pointer layouter = mitk::Overlay2DLayouter::New();
    layouter->m_Alignment = alignment;
    layouter->m_Identifier = identifier;
    return layouter;
  }

  void PrepareLayout();
protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit Overlay2DLayouter();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~Overlay2DLayouter();

  Alignment m_Alignment;

private:

  /** \brief copy constructor */
  Overlay2DLayouter( const Overlay2DLayouter &);

  /** \brief assignment operator */
  Overlay2DLayouter &operator=(const Overlay2DLayouter &);
};

} // namespace mitk
#endif // MITKOVERLAY2DLAYOUTER_H


