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


#ifndef MITKSURFACEDATASOURCE_H_HEADER_INCLUDED_C10B4740
#define MITKSURFACEDATASOURCE_H_HEADER_INCLUDED_C10B4740

#include "mitkBaseProcess.h"

namespace mitk {
class Surface;
//##ModelId=3EF4A43301E0
//##Documentation
//## @brief Superclass of all classes generating surfaces (instances of class
//## Surface) as output. 
//##
//## In itk and vtk the generated result of a ProcessObject is only guaranteed
//## to be up-to-date, when Update() of the ProcessObject or the generated
//## DataObject is called immediately before access of the data stored in the
//## DataObject. This is also true for subclasses of mitk::BaseProcess and thus
//## for mitk::SurfaceSource.
//## @ingroup Process
class SurfaceSource : public BaseProcess
{
public:
  mitkClassMacro(SurfaceSource, BaseProcess);

  itkNewMacro(Self);  

  //##ModelId=3EF56D670290
  typedef itk::DataObject::Pointer DataObjectPointer;

  //##ModelId=3EF56B0703A4
  virtual DataObjectPointer MakeOutput(unsigned int idx);


  //##ModelId=3EF56B1101F0
  void SetOutput(mitk::Surface* output);

  //##ModelId=3EF56B16028D
  mitk::Surface* GetOutput();

  //##ModelId=3EF56B1A0257
  mitk::Surface* GetOutput(unsigned int idx);

  //##ModelId=3EF56B1303D3
  virtual void GraftOutput(mitk::Surface* graft);

  //##ModelId=3EF56C440197
  virtual void GraftNthOutput(unsigned int idx, mitk::Surface *graft);

protected:
  //##ModelId=3EF4A4A3001E
  SurfaceSource();

  //##ModelId=3EF4A4A3003C
  virtual ~SurfaceSource();
};

} // namespace mitk



#endif /* MITKSURFACEDATASOURCE_H_HEADER_INCLUDED_C10B4740 */
