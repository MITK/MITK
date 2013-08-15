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


#ifndef mitkDicomRTReader_h
#define mitkDicomRTReader_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkDicomRTExports.h>

#include <gdcmTag.h>
#include <gdcmScanner.h>
#include "gdcmReader.h"
#include "gdcmAttribute.h"
#include "gdcmSequenceOfItems.h"
#include "gdcmFileExplicitFilter.h"


namespace mitk
{

class mitkDicomRT_EXPORT DicomRTReader: public itk::Object
{
public:
  mitkClassMacro( DicomRTReader, itk::Object );

  itkNewMacro( Self );

  int ReadContourDataSequence0(char* filename);
  int ReadContourDataSequence0_1(char* filename);
  int ReadContourDataSequence1(char* filename);
  int ReadContourDataSequence2(char* filename);
  int ReadContourDataSequence3(char* filename);

  /**
  * Virtual destructor.
  */
  virtual ~DicomRTReader();

protected:

  /**
  * Constructor.
  */
  DicomRTReader();

};

}

#endif