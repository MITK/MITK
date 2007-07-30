/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 5308 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_UNSTRUCTURED_GRID_DATA_SOURCE_H_HEADER_
#define _MITK_UNSTRUCTURED_GRID_DATA_SOURCE_H_HEADER_

#include "mitkBaseProcess.h"

namespace mitk {
class UnstructuredGrid;
//##Documentation
//## @brief Superclass of all classes generating unstructured grids (instances of class
//## UnstructuredGrid) as output. 
//##
//## In itk and vtk the generated result of a ProcessObject is only guaranteed
//## to be up-to-date, when Update() of the ProcessObject or the generated
//## DataObject is called immediately before access of the data stored in the
//## DataObject. This is also true for subclasses of mitk::BaseProcess and thus
//## for mitk::UnstructuredGridSource.
//## @ingroup Process
class UnstructuredGridSource : public BaseProcess
{
public:
  mitkClassMacro(UnstructuredGridSource, BaseProcess);

  itkNewMacro(Self);  

  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(unsigned int idx);


  void SetOutput(mitk::UnstructuredGrid* output);

  mitk::UnstructuredGrid* GetOutput();

  mitk::UnstructuredGrid* GetOutput(unsigned int idx);

  virtual void GraftOutput(mitk::UnstructuredGrid* graft);

  virtual void GraftNthOutput(unsigned int idx, mitk::UnstructuredGrid *graft);

protected:
  UnstructuredGridSource();

  virtual ~UnstructuredGridSource();
};

} // namespace mitk



#endif /* _MITK_UNSTRUCTURED_GRID_DATA_SOURCE_H_HEADER_ */
