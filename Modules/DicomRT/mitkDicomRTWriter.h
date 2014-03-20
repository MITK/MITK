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


#ifndef mitkDicomRTWriter_h
#define mitkDicomRTWriter_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <mitkDicomRTExports.h>

namespace mitk
{

/**
 * @brief bla
 *
 */
class mitkDicomRT_EXPORT DicomRTWriter: public itk::Object
{
public:

  mitkClassMacro( DicomRTWriter, itk::Object );

  itkNewMacro( Self );

  std::string SayHelloWorld();

  /**
  * Virtual destructor.
  */
  virtual ~DicomRTWriter();

protected:

  /**
  * Constructor.
  */
  DicomRTWriter();

};



}


#endif
