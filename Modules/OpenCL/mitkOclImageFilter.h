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

#ifndef __mitkOclImageFilter_h
#define __mitkOclImageFilter_h

#include "mitkOclFilter.h"
#include "mitkOclImage.h"

#define FILTER_UCHAR 0
#define FILTER_SHORT 1

namespace mitk
{
class OclFilter;
class OclImageFilter;

/**
    * \brief The OclImageFilter is the topmost class for all filter which takes an image as input
    *
    * The input image can be intialized by an oclImage or an mitk::Image.
    * This makes it possible to create a filter pipeline of GPU-based filters
    * and to bind this part into the CPU (ITK) filter pipeline
    */
class MitkOcl_EXPORT OclImageFilter: public OclFilter
{
public:
  /**
     \brief Set the input image ( oclImage )
    */
  void SetInput(mitk::OclImage::Pointer );

  /**
      \brief Set the input image ( mitkImage )
      */
  void SetInput(mitk::Image::Pointer);

  virtual ~OclImageFilter();

protected:
  OclImageFilter();

  /** The input image */
  mitk::OclImage::Pointer m_input;

  /*! Current (last used) image pixel type */
  short m_currentType;


};
}
#endif // __mitkOclImageFilter_h
