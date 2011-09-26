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

#include "mitkNrrdTbssImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdTbssImageWriter.h>

namespace mitk
{

NrrdTbssImageWriterFactory::NrrdTbssImageWriterFactory()
{
  typedef float TbssPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdTbssImageWriter",
                         "NrrdTbssImage Writer",
                         1,
                         itk::CreateObjectFunction< mitk::NrrdTbssImageWriter >::New());
}

NrrdTbssImageWriterFactory::~NrrdTbssImageWriterFactory()
{
}

const char* NrrdTbssImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdTbssImageWriterFactory::GetDescription() const
{
  return "NrrdTbssImageWriterFactory";
}

} // end namespace mitk
