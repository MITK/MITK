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


#ifndef MITKGEOMETRYDATASOURCE_H_HEADER_INCLUDED_C10B4740
#define MITKGEOMETRYDATASOURCE_H_HEADER_INCLUDED_C10B4740

#include "mitkBaseDataSource.h"
#include "MitkAlgorithmsExtExports.h"

namespace mitk {

class GeometryData;

/**
 * @brief Superclass of all classes generating GeometryData (instances of class
 * GeometryData) as output.
 *
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. This is also true for subclasses of mitk::BaseProcess and thus
 * for mitk::GeometryDataSource.
 * @ingroup Process
 */
class MITKALGORITHMSEXT_EXPORT GeometryDataSource : public BaseDataSource
{
public:

  mitkClassMacro(GeometryDataSource, BaseDataSource)
  itkNewMacro(Self)

  typedef mitk::GeometryData OutputType;

  mitkBaseDataSourceGetOutputDeclarations

  /**
   * Allocates a new output object and returns it. Currently the
   * index idx is not evaluated.
   * @param idx the index of the output for which an object should be created
   * @returns the new object
   */
  virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx ) override;

  /**
   * This is a default implementation to make sure we have something.
   * Once all the subclasses of ProcessObject provide an appopriate
   * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
   * virtual.
   */
  virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

protected:

  GeometryDataSource();
  virtual ~GeometryDataSource();
};

} // namespace mitk



#endif /* MITKGEOMETRYDATASOURCE_H_HEADER_INCLUDED_C10B4740 */
