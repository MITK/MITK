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


#ifndef MITKITKPICTUREWRITE_H
#define MITKITKPICTUREWRITE_H

 #include <itkImage.h>

template < typename TPixel, unsigned int VImageDimension > 
void _mitkItkPictureWrite(itk::Image< TPixel, VImageDimension >* itkImage, const std::string& fileName);

#endif /* MITKITKPICTUREWRITE_H */
