/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkIOAdapter_h
#define __mitkIOAdapter_h

#include "mitkBaseProcess.h"

#include "itkLightObject.h"
#include "itkObject.h"


namespace mitk
{

//## @brief IOAdapterBase class is an abstract adapter class for IO process objects. 
//##
//## @ingroup IO
class IOAdapterBase: public itk::Object
{
public:
  /** Standard typedefs. */
  typedef IOAdapterBase Self;
  typedef itk::Object Superclass;
  typedef itk::SmartPointer<Self>Pointer;
  typedef itk::SmartPointer<const Self>ConstPointer;
  
  /** Create an object and return a pointer to it as an
   * itk::LightObject. */
  virtual itk::SmartPointer<BaseProcess> CreateIOProcessObject(const char* filename) = 0;
  virtual bool CanReadFile(const char* filename) = 0;

protected:
  IOAdapterBase() {}
  ~IOAdapterBase() {}
  
private:
  IOAdapterBase(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented  
};


//## @brief IOAdapter class is an adapter class for instantiation of IO process objects.
//## Additional this interface defines the function CanReadFile(). 
//## This interface allows the target (object) the access to the adaptee (IO process object).
//## @ingroup IO
template <class T>
class IOAdapter : public IOAdapterBase
{
public:
  /** Standard class typedefs. */
  typedef IOAdapter Self;
  typedef itk::SmartPointer<Self> Pointer;
    
  /** Methods from itk:LightObject. */
  itkFactorylessNewMacro(Self);

  mitk::BaseProcess::Pointer CreateIOProcessObject(const char* filename)
  { 
    typename T::Pointer ioProcessObject = T::New();
    ioProcessObject->SetFileName(filename);
    return ioProcessObject.GetPointer(); 
  }
  
  virtual bool CanReadFile(const char* filename)
  {
    return T::CanReadFile(filename);
  }

protected:
  IOAdapter() {}
  ~IOAdapter() {}
  
private:
  IOAdapter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented  
};


} // end namespace itk

#endif
