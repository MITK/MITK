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


#ifndef TensorImageSource_H_HEADER_INCLUDED_C1E7D6EC
#define TensorImageSource_H_HEADER_INCLUDED_C1E7D6EC

#include "mitkImageSource.h"
#include "mitkTensorImage.h"

namespace mitk {

class TensorImageSource : public ImageSource
{
public:

  typedef mitk::TensorImage OutputImageType;
  typedef OutputImageType::Pointer OutputImagePointer;
  typedef SlicedData::RegionType OutputImageRegionType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  mitkClassMacro(TensorImageSource,ImageSource);
  itkNewMacro(Self);

  virtual DataObjectPointer MakeOutput(unsigned int idx);

protected:
  TensorImageSource();
  virtual ~TensorImageSource() {}

};

} // namespace mitk

#endif /* TensorImageSource_H_HEADER_INCLUDED_C1E7D6EC */
