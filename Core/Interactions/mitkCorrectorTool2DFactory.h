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
#ifndef __mitkCorrectorTool2DFactory_h
#define __mitkCorrectorTool2DFactory_h

#include "itkObjectFactoryBase.h"

namespace mitk
{

/**
  \brief Factory to create a mitk::AddContourTool.

  \section short explanation on all these factories.

  There is a ITK factory for each single mitk::Tool. There are two questions that might be asked: "why factories?" and "why one for every class?".

  \subsection Why factories at all?

  Factories reduce compiler dependencies. A lot of classes can be created (and used) by only knowing their interface's header file.
  There is still one location where the factories have to be registered and all those header files have to be included, 
  but, first, these header files are minimal, and second, this is only one single location in the whole project.

  \subsection Why is there a separate factory for each tool?

  This is due to a limitation of the ITK object factories. ObjectFactoryBase::CreateAllInstance(...) asks each factory
  for only one single object of a certain class.

  $Author$
*/
class CorrectorTool2DFactory : public itk::ObjectFactoryBase
{
  public:  
    /// ITK typedefs
    typedef CorrectorTool2DFactory   Self;
    typedef itk::ObjectFactoryBase  Superclass;
    typedef itk::SmartPointer<Self>  Pointer;
    typedef itk::SmartPointer<const Self>  ConstPointer;
     
    /// Methods from ObjectFactoryBase
    virtual const char* GetITKSourceVersion() const;
    virtual const char* GetDescription() const;
    
    /// Method for class instantiation.
    itkFactorylessNewMacro(Self);
      
    /// Run-time type information (and related methods).
    itkTypeMacro(CorrectorTool2DFactory, itkObjectFactoryBase);

    /** Register one factory of this type  */
    static void RegisterOneFactory()
    {
      CorrectorTool2DFactory::Pointer CorrectorTool2DFactory = CorrectorTool2DFactory::New();
      ObjectFactoryBase::RegisterFactory(CorrectorTool2DFactory);
    }
    
  protected:

    CorrectorTool2DFactory();
    ~CorrectorTool2DFactory();

  private:

    CorrectorTool2DFactory(const Self&); //purposely not implemented
    void operator=(const Self&);    //purposely not implemented

};
  
  
} // end namespace mitk

#endif

