/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkLightBoxImageReader.h"
#include "mitkPlaneGeometry.h"
#include "mitkFrameOfReferenceUIDManager.h"
#include <mitkChiliPlugin.h>
#include <itkImageFileReader.h>
#include <list>
#include <vnl/vnl_cross.h>
#include <stdlib.h>

void mitk::LightBoxImageReader::SetLightBox(QcLightbox*)
{
}
void mitk::LightBoxImageReader::SetLightBoxToCurrentLightBox()
{
}

const std::string mitk::LightBoxImageReader::GetSeriesDescription()
{
  std::string result;
  return result;
}

QcLightbox* mitk::LightBoxImageReader::GetLightBox() const
{
  return NULL;
}

