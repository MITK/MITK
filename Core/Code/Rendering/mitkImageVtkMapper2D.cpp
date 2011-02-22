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


#include "mitkImageVtkMapper2D.h"
//#include "mitkMapper.h"
//#include "mitkDataNode.h"
//#include "mitkBaseRenderer.h"
//#include "mitkProperties.h"

mitk::ImageVtkMapper2D::ImageVtkMapper2D()
{
  this->m_VtkBased = true;
//  this->m_TimeStep = 0;
}


mitk::ImageVtkMapper2D::~ImageVtkMapper2D()
{
}
