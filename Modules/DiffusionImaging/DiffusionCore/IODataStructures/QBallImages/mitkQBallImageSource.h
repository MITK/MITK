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


#ifndef QBallImageSource_H_HEADER_INCLUDED_C1E7D6EC
#define QBallImageSource_H_HEADER_INCLUDED_C1E7D6EC

#include "mitkImageSource.h"
#include "mitkQBallImage.h"
#include <DiffusionCoreExports.h>

namespace mitk {

class DiffusionCore_EXPORT QBallImageSource : public ImageSource
{
public:

  typedef mitk::QBallImage OutputImageType;
  typedef OutputImageType::Pointer OutputImagePointer;
  typedef SlicedData::RegionType OutputImageRegionType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  mitkClassMacro(QBallImageSource,ImageSource);
  itkNewMacro(Self);

  virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx);

protected:
  QBallImageSource();
  virtual ~QBallImageSource() {}

};

} // namespace mitk

#endif /* QBallImageSource_H_HEADER_INCLUDED_C1E7D6EC */
