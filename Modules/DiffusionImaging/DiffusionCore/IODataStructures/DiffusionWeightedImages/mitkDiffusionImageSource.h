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


#ifndef _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_
#define _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_

#include "mitkImageSource.h"

namespace mitk {

  template<typename TPixelType>
  class DiffusionImage;
  /*class DiffusionImage<double>;
  class DiffusionImage<int>;
  class DiffusionImage<unsigned int>;
  class DiffusionImage<short>;
  class DiffusionImage<unsigned short>;
  class DiffusionImage<char>;
  class DiffusionImage<unsigned char>;
  class DiffusionImage<long>;
  class DiffusionImage<unsigned long>;*/

//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances
//## of class DiffusionImage) as output.
//##
//## @ingroup Process
template<typename TPixelType>
class DiffusionImageSource : public ImageSource
{
public:
  mitkClassMacro(DiffusionImageSource, BaseDataSource);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef DiffusionImage<TPixelType> OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  /**
    * @brief Get the output data of the diffusion image source object.
    */
  mitkBaseDataSourceGetOutputDeclarations

  /**
   * Allocates a new output object and returns it. Currently the
   * index idx is not evaluated.
   * @param idx the index of the output for which an object should be created
   * @returns the new object
   */
  virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx );

  /**
   * This is a default implementation to make sure we have something.
   * Once all the subclasses of ProcessObject provide an appopriate
   * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
   * virtual.
   */
  virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name);

protected:
  DiffusionImageSource();

  virtual ~DiffusionImageSource();
};

} // namespace mitk

#include "mitkDiffusionImageSource.cpp"


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
