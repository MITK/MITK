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


#ifndef MITKGEOMETRYDATASOURCE_H_HEADER_INCLUDED_C10B4740
#define MITKGEOMETRYDATASOURCE_H_HEADER_INCLUDED_C10B4740

#include "mitkBaseProcess.h"
#include "mitkGeometryDataSource.h"

namespace mitk {

class GeometryData;

//##Documentation
//## @brief Superclass of all classes generating GeometryData (instances of class
//## GeometryData) as output. 
//##
//## In itk and vtk the generated result of a ProcessObject is only guaranteed
//## to be up-to-date, when Update() of the ProcessObject or the generated
//## DataObject is called immediately before access of the data stored in the
//## DataObject. This is also true for subclasses of mitk::BaseProcess and thus
//## for mitk::GeometryDataSource.
//## @ingroup Process
class MITK_CORE_EXPORT GeometryDataSource : public BaseProcess
{
public:
  mitkClassMacro(GeometryDataSource, BaseProcess);

  itkNewMacro(Self);  

  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(unsigned int idx);

  void SetOutput(mitk::GeometryData* output);

  mitk::GeometryData* GetOutput();

  mitk::GeometryData* GetOutput(unsigned int idx);

  virtual void GraftOutput(mitk::GeometryData* graft);

  virtual void GraftNthOutput(unsigned int idx, mitk::GeometryData *graft);

protected:
  GeometryDataSource();

  virtual ~GeometryDataSource();
};

} // namespace mitk



#endif /* MITKGEOMETRYDATASOURCE_H_HEADER_INCLUDED_C10B4740 */
