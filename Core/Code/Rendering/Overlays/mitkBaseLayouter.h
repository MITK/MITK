/*===================================================================
 *
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BASELAYOUTER_H
#define BASELAYOUTER_H

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>

namespace mitk {

class BaseLayouter : public itk::Object {
public:

  mitkClassMacro(BaseLayouter, itk::Object);

protected:

  /** \brief explicit constructor which disallows implicit conversions */
  explicit BaseLayouter();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~BaseLayouter();

private:

  /** \brief copy constructor */
  BaseLayouter( const BaseLayouter &);

  /** \brief assignment operator */
  BaseLayouter &operator=(const BaseLayouter &);
};

} // namespace mitk
#endif // BASELAYOUTER_H


