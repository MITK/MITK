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

#ifndef MITKOVERLAYLAYOUTER_H
#define MITKOVERLAYLAYOUTER_H

#include <MitkExports.h>
#include <itkObject.h>
#include "mitkBaseLayouter.h"

namespace mitk {

class OverlayLayouter : public mitk::BaseLayouter {
public:

  mitkClassMacro(OverlayLayouter, mitk::BaseLayouter);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit OverlayLayouter();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~OverlayLayouter();

private:

  /** \brief copy constructor */
  OverlayLayouter( const OverlayLayouter &);

  /** \brief assignment operator */
  OverlayLayouter &operator=(const OverlayLayouter &);
};

} // namespace mitk
#endif // MITKOVERLAYLAYOUTER_H


