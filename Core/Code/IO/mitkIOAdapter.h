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
#ifndef __mitkIOAdapter_h
#define __mitkIOAdapter_h

#include "mitkBaseProcess.h"

#include "itkObject.h"


namespace mitk
{
//##Documentation
//## @brief IOAdapterBase class is an abstract adapter class for IO process objects.
//##
//## @ingroup IO
//## @deprecatedSince{2014_10} Use mitk::IFileReader instead
class IOAdapterBase: public itk::Object
{
public:
  /** Standard typedefs. */
  typedef IOAdapterBase Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self>Pointer;
  typedef itk::SmartPointer<const Self>ConstPointer;

  /// Create an object and return a pointer to it as a mitk::BaseProcess.
  virtual itk::SmartPointer<BaseDataSource> CreateIOProcessObject(const std::string filename, const std::string filePrefix, const std::string filePattern) = 0;
  virtual bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern) = 0;

protected:
  IOAdapterBase() {}
  ~IOAdapterBase() {}

private:
  IOAdapterBase(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

//##Documentation
//## @brief IOAdapter class is an adapter class for instantiation of IO process objects.
//## Additional this interface defines the function CanReadFile().
//## This interface allows the target (object) the access to the adaptee (IO process object).
//## @ingroup IO
//## @deprecatedSince{2014_10} Use mitk::IFileReader instead
template <class T>
class IOAdapter : public IOAdapterBase
{
public:
  /** Standard class typedefs. */
  typedef IOAdapter Self;
  typedef itk::SmartPointer<Self> Pointer;

  /** Methods from mitk::BaseProcess. */
  itkFactorylessNewMacro(Self);
  mitk::BaseProcess::Pointer CreateIOProcessObject(const std::string filename, const std::string filePrefix, const std::string filePattern)
  {
    typename T::Pointer ioProcessObject = T::New();
    ioProcessObject->SetFileName(filename.c_str());
    ioProcessObject->SetFilePrefix(filePrefix.c_str());
    ioProcessObject->SetFilePattern(filePattern.c_str());
    return ioProcessObject.GetPointer();
  }

  virtual bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern)
  {
    return T::CanReadFile(filename, filePrefix, filePattern);
  }

protected:
  IOAdapter() {}
  ~IOAdapter() {}

private:
  IOAdapter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};


} // end namespace mitk

#endif
