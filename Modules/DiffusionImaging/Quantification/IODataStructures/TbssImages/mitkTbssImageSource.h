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


#ifndef _MITK_TBSS_IMAGE_DATA_SOURCE_H_HEADER_
#define _MITK_TBSS_IMAGE_DATA_SOURCE_H_HEADER_


// Should be changed in a new type for TBSS
#include "mitkImageSource.h"
#include "QuantificationExports.h"


namespace mitk {



  class TbssImage;


//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances
//## of class DiffusionImage) as output.
//##
//## @ingroup Process

class Quantification_EXPORT TbssImageSource : public ImageSource
{
public:
  mitkClassMacro(TbssImageSource, BaseProcess)
  itkNewMacro(Self)

  typedef TbssImage OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx);

  OutputType* GetOutput()
  {
      return itkDynamicCastInDebugMode<OutputType*>( this->GetPrimaryOutput() );
  }

  //OutputType * GetOutput(unsigned int idx);

  //void SetOutput(OutputType* output);

  //{return Superclass::GetOutput();}

  OutputType* GetOutput(unsigned int idx);

  //virtual void GraftOutput(OutputType* graft);

  //virtual void GraftNthOutput(unsigned int idx, OutputType *graft);

protected:
  TbssImageSource();

  virtual ~TbssImageSource(){}
};

} // namespace mitk

//#include "mitkTbssImageSource.cpp"


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
