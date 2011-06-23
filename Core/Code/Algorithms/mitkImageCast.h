/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKIMAGECAST_H_HEADER_INCLUDED
#define MITKIMAGECAST_H_HEADER_INCLUDED

#include <mitkCommon.h>
#include <mitkImage.h>

#include <itkImage.h>

namespace mitk
{

#ifndef DOXYGEN_SKIP
  template < typename TPixel, unsigned int VImageDimension, class ItkOutputImageType >
  void _CastToItkImage2Access( itk::Image<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);
#endif //DOXYGEN_SKIP

 //##Documentation
 //## @brief Cast an mitk::Image to an itk::Image with a specific type. You don't have to initialize the itk::Image<..>::Pointer.
 //## @ingroup Adaptor
 template <typename ItkOutputImageType> extern void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);
}

#endif // of MITKIMAGECAST_H_HEADER_INCLUDED
