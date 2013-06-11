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

class Overlay2DLayouter : public mitk::BaseLayouter {
public:

  mitkClassMacro(Overlay2DLayouter, mitk::BaseLayouter);
  enum Layout {TopLeft, Top, TopRight, Left, Middle, Right, BottomLeft, Bottom, BottomRight};

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit Overlay2DLayouter();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~Overlay2DLayouter();

private:

  /** \brief copy constructor */
  Overlay2DLayouter( const Overlay2DLayouter &);

  /** \brief assignment operator */
  Overlay2DLayouter &operator=(const Overlay2DLayouter &);
};

} // namespace mitk
#endif // MITKOVERLAY2DLAYOUTER_H


