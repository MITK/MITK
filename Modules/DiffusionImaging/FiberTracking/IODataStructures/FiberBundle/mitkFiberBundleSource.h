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


#ifndef OdfImageSource_H_HEADER_INCLUDED_C1E7D6EC
#define OdfImageSource_H_HEADER_INCLUDED_C1E7D6EC

#include "mitkImageSource.h"
#include "mitkOdfImage.h"

//NOTE>umbenennen .. ableiten von DataSource


namespace mitk {

class OdfImageSource : public ImageSource
{
public:

  typedef mitk::OdfImage OutputImageType;
  typedef OutputImageType::Pointer OutputImagePointer;
  typedef SlicedData::RegionType OutputImageRegionType;
  typedef itk::DataObject::Pointer DataObjectPointer;

  mitkClassMacro(OdfImageSource,ImageSource);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual DataObjectPointer MakeOutput(DataObjectPointerArraySizeType idx);

protected:
  OdfImageSource();
  virtual ~OdfImageSource() {}

};

} // namespace mitk

#endif /* OdfImageSource_H_HEADER_INCLUDED_C1E7D6EC */
