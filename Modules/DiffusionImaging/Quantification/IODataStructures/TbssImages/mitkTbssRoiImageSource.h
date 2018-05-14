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


#ifndef _MITK_TBSS_ROI_IMAGE_DATA_SOURCE_H_HEADER_
#define _MITK_TBSS_ROI_IMAGE_DATA_SOURCE_H_HEADER_


// Should be changed in a new type for TBSS
#include "mitkImageSource.h"
#include "MitkQuantificationExports.h"


namespace mitk {



  class TbssRoiImage;


//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances
//## of class DiffusionImage) as output.
//##
//## @ingroup Process

class MITKQUANTIFICATION_EXPORT TbssRoiImageSource : public ImageSource
{
public:
  mitkClassMacro(TbssRoiImageSource, BaseDataSource)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef TbssRoiImage OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  /**
   * Allocates a new output object and returns it. Currently the
   * index idx is not evaluated.
   * @param idx the index of the output for which an object should be created
   * @returns the new object
   */
  itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx ) override;

  /**
   * This is a default implementation to make sure we have something.
   * Once all the subclasses of ProcessObject provide an appopriate
   * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
   * virtual.
   */
  itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

  OutputType* GetOutput(unsigned int idx);

protected:
  TbssRoiImageSource();

  ~TbssRoiImageSource() override{}
};

} // namespace mitk


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
