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
  mitkClassMacro(DiffusionImageSource, BaseProcess);
  itkNewMacro(Self);

  typedef DiffusionImage<TPixelType> OutputType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  virtual DataObjectPointer MakeOutput(unsigned int idx);


  //void SetOutput(OutputType* output);

  //OutputType* GetOutput()
  //{return Superclass::GetOutput();}

  //OutputType* GetOutput(unsigned int idx);

  //virtual void GraftOutput(OutputType* graft);

  //virtual void GraftNthOutput(unsigned int idx, OutputType *graft);

protected:
  DiffusionImageSource();

  virtual ~DiffusionImageSource();
};

} // namespace mitk

#include "mitkDiffusionImageSource.cpp"


#endif /* _MITK_DIFFUSION_IMAGE_DATA_SOURCE_H_HEADER_ */
