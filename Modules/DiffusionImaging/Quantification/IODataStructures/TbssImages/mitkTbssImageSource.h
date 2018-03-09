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
#include "MitkQuantificationExports.h"


namespace mitk {



  class TbssImage;


//##Documentation
//## @brief Superclass of all classes generating diffusion volumes (instances
//## of class DiffusionImage) as output.
//##
//## @ingroup Process

class MITKQUANTIFICATION_EXPORT TbssImageSource : public ImageSource
{
public:
  mitkClassMacro(TbssImageSource, BaseDataSource)
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  typedef TbssImage OutputType;
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

  ~TbssImageSource() override{}
};

} // namespace mitk

//#include "mitkTbssImageSource.cpp"


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
