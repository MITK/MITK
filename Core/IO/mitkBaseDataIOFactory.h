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

#ifndef __mitkBaseDataIOFactory_h
#define __mitkBaseDataIOFactory_h

#include "mitkBaseData.h"

#include "itkObject.h"

namespace mitk
{

//## @brief BaseDataIOFactory class creates instances of BaseDataIO objects using an object factory.
//##
//## @ingroup IO
class BaseDataIOFactory : public itk::Object
{
public:  
  /** Standard class typedefs. */
  typedef BaseDataIOFactory   Self;
  typedef itk::Object  Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;
  
  /** Class Methods used to interface with the registered factories */
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(BaseDataIOFactory, Object);

  /** Convenient typedefs. */
  //typedef ::mitk::IOAdapter::Pointer ImageIOBasePointer;

  /** Mode in which the files is intended to be used */
  typedef enum { ReadMode, WriteMode } FileModeType;
  
  /** Create the appropriate BaseData depending on the particulars of the file. */
  static mitk::BaseData::Pointer CreateBaseDataIO(const char* path, FileModeType mode);

  /** Register Built-in factories */
  static void RegisterBuiltInFactories();

protected:
  BaseDataIOFactory();
  ~BaseDataIOFactory();

private:
  BaseDataIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
  
};
  
  
} // end namespace mitk

#endif
