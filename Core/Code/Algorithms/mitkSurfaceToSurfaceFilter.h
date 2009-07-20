/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

  virtual void SetInput( const mitk::Surface* surface );

  virtual void SetInput( unsigned int idx, const mitk::Surface* surface );

  virtual const mitk::Surface* GetInput();

  virtual const mitk::Surface* GetInput( unsigned int idx );

  virtual void CreateOutputsForAllInputs(unsigned int idx);

  virtual void RemoveInputs(mitk::Surface* input);

protected:
  SurfaceToSurfaceFilter();

  virtual ~SurfaceToSurfaceFilter();
};

} // namespace mitk



#endif /* MITKSURFACETOSURFACEFILTER_H_HEADER_INCLUDED_C10B4740 */
