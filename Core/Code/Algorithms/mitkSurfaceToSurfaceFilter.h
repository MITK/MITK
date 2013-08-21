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


#ifndef MITKSURFACETOSURFACEFILTER_H_HEADER_INCLUDED_C10B4740
#define MITKSURFACETOSURFACEFILTER_H_HEADER_INCLUDED_C10B4740

#include "mitkSurfaceSource.h"

namespace mitk {
class Surface;
//##Documentation
//## @brief Superclass of all classes getting surfaces (instances of class
//## Surface) as input and generating surfaces as output.
//##
//## In itk and vtk the generated result of a ProcessObject is only guaranteed
//## to be up-to-date, when Update() of the ProcessObject or the generated
//## DataObject is called immediately before access of the data stored in the
//## DataObject. This is also true for subclasses of mitk::BaseProcess and thus
//## for mitk::mitkSurfaceToSurfaceFilter.
//## @ingroup Process
class MITK_CORE_EXPORT SurfaceToSurfaceFilter : public mitk::SurfaceSource
{
public:
  mitkClassMacro(SurfaceToSurfaceFilter, mitk::SurfaceSource);

  itkNewMacro(Self);

  typedef itk::DataObject::Pointer DataObjectPointer;

  using itk::ProcessObject::SetInput;
  virtual void SetInput( const mitk::Surface* surface );

  virtual void SetInput( unsigned int idx, const mitk::Surface* surface );

  virtual const mitk::Surface* GetInput();

  virtual const mitk::Surface* GetInput( unsigned int idx );

  virtual void CreateOutputsForAllInputs(unsigned int idx);

protected:
  SurfaceToSurfaceFilter();

  virtual ~SurfaceToSurfaceFilter();
};

} // namespace mitk



#endif /* MITKSURFACETOSURFACEFILTER_H_HEADER_INCLUDED_C10B4740 */
