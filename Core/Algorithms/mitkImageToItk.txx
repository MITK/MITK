/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef IMAGETOITK_TXX_INCLUDED_C1C2FCD2
#define IMAGETOITK_TXX_INCLUDED_C1C2FCD2

#include "mitkImageToItk.h"

namespace mitk
{

template<class TPixel, unsigned int VImageDimension>
ImageToItk<TPixel, VImageDimension>
::ImageToItk() : itk::Image<TPixel, VImageDimension>(), m_CopyMemFlag(false)
{

}


template<class TPixel, unsigned int VImageDimension>
ImageToItk<TPixel, VImageDimension>
::~ImageToItk()
{

}

template<class TPixel, unsigned int VImageDimension>
void
ImageToItk<TPixel, VImageDimension>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);

	std::cout << "Mit2itkWrapper .. " << std::endl;
}

}
#endif //IMAGETOITK_TXX_INCLUDED_C1C2FCD2
