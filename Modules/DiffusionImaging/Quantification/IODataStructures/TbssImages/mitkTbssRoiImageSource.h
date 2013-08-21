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
#include "QuantificationExports.h"


namespace mitk {



  class TbssRoiImage;


//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances
//## of class DiffusionImage) as output.
//##
//## @ingroup Process

class Quantification_EXPORT TbssRoiImageSource : public ImageSource
{
public:
  mitkClassMacro(TbssRoiImageSource, BaseProcess)
  itkNewMacro(Self)

  typedef TbssRoiImage OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx);

  OutputType* GetOutput(unsigned int idx);

protected:
  TbssRoiImageSource();

  virtual ~TbssRoiImageSource(){}
};

} // namespace mitk


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
