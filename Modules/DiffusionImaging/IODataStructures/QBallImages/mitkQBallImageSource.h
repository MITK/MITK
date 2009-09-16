/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef QBallImageSource_H_HEADER_INCLUDED_C1E7D6EC
#define QBallImageSource_H_HEADER_INCLUDED_C1E7D6EC

#include "mitkImageSource.h"
#include "mitkQBallImage.h"

namespace mitk {

class QBallImageSource : public ImageSource
{
public:

  typedef mitk::QBallImage OutputImageType;
  typedef OutputImageType::Pointer OutputImagePointer;
  typedef SlicedData::RegionType OutputImageRegionType;
  typedef itk::DataObject::Pointer DataObjectPointer;
 
  mitkClassMacro(QBallImageSource,ImageSource);
  itkNewMacro(Self);  

  virtual DataObjectPointer MakeOutput(unsigned int idx);

protected:
  QBallImageSource();
  virtual ~QBallImageSource() {}
  
};

} // namespace mitk

#endif /* QBallImageSource_H_HEADER_INCLUDED_C1E7D6EC */
