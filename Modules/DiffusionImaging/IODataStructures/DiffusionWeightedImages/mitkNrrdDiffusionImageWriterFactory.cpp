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

#include "mitkNrrdDiffusionImageWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkNrrdDiffusionImageWriter.h>

namespace mitk
{

template <class T>
class CreateNrrdDiffusionImageWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreateNrrdDiffusionImageWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreateNrrdDiffusionImageWriter() {}
  ~CreateNrrdDiffusionImageWriter() {}

private:
  CreateNrrdDiffusionImageWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

NrrdDiffusionImageWriterFactory::NrrdDiffusionImageWriterFactory()
{
  typedef short DiffusionPixelType;
  this->RegisterOverride("IOWriter",
                         "NrrdDiffusionImageWriter",
                         "NrrdDiffusionImage Writer",
                         1,
                         mitk::CreateNrrdDiffusionImageWriter< mitk::NrrdDiffusionImageWriter<DiffusionPixelType> >::New());
}

NrrdDiffusionImageWriterFactory::~NrrdDiffusionImageWriterFactory()
{
}

const char* NrrdDiffusionImageWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* NrrdDiffusionImageWriterFactory::GetDescription() const
{
  return "NrrdDiffusionImageWriterFactory";
}

} // end namespace mitk
