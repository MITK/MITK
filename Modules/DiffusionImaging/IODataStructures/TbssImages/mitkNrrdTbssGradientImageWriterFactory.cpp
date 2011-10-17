/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNrrdTbssGradientImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdTbssGradientImageWriter.h>

namespace mitk
{

NrrdTbssGradientImageWriterFactory::NrrdTbssGradientImageWriterFactory()
{
  typedef int TbssGradientPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdTbssGradientImageWriter",
                         "NrrdTbssGradientImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdTbssGradientImageWriter >::New());
}

NrrdTbssGradientImageWriterFactory::~NrrdTbssGradientImageWriterFactory()
{
}

const char* NrrdTbssGradientImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTbssGradientImageWriterFactory::GetDescription() const
{
  return "NrrdTbssGradientImageWriterFactory";
}

} // end namespace mitk
