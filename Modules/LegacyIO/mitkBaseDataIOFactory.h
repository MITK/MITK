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

#ifndef __mitkBaseDataIO_h
#define __mitkBaseDataIO_h

#include <MitkLegacyIOExports.h>

#include "mitkBaseData.h"

#include "itkObject.h"

namespace mitk
{

/*!
 @brief BaseDataIO creates instances of BaseData objects using an object factory.

 @todo Add file writing method, move writers into a similar factory scheme
 @ingroup IO
 @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
*/
class DEPRECATED() MitkLegacyIO_EXPORT BaseDataIO : public itk::Object
{
public:

  /** Standard class typedefs. */
  typedef BaseDataIO   Self;
  typedef itk::Object  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Class Methods used to interface with the registered factories */

  /** Run-time type information (and related methods). */
  itkTypeMacro(BaseDataIO, Object);

  /** Create the appropriate BaseData depending on the particulars of the file. */
  static std::vector<mitk::BaseData::Pointer> LoadBaseDataFromFile(const std::string path, const std::string filePrefix, const std::string filePattern, bool series);

protected:
  BaseDataIO();
  ~BaseDataIO();

private:
  BaseDataIO(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace mitk

#endif
