/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11. Dez 2007) $
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPlanarFigureWriterFactory.h"

#include "itkCreateObjectFunction.h"
#include "itkVersion.h"

#include <mitkPlanarFigureWriter.h>

namespace mitk
{

template <class T>
class CreatePlanarFigureWriter : public itk::CreateObjectFunctionBase
{
public:

  /** Standard class typedefs. */
  typedef CreatePlanarFigureWriter  Self;
  typedef itk::SmartPointer<Self>    Pointer;

  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);
  LightObject::Pointer CreateObject() { typename T::Pointer p = T::New();
    p->Register();
    return p.GetPointer();
  }

protected:
  CreatePlanarFigureWriter() {}
  ~CreatePlanarFigureWriter() {}

private:
  CreatePlanarFigureWriter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

PlanarFigureWriterFactory::PlanarFigureWriterFactory()
{
  this->RegisterOverride("IOWriter",
                         "PlanarFigureWriter",
                         "PlanarFigure xml Writer",
                         1,
                         mitk::CreatePlanarFigureWriter< mitk::PlanarFigureWriter >::New());
}

PlanarFigureWriterFactory::~PlanarFigureWriterFactory()
{
}

const char* PlanarFigureWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PlanarFigureWriterFactory::GetDescription() const
{
  return "PlanarFigureWriterFactory";
}

} // end namespace mitk
