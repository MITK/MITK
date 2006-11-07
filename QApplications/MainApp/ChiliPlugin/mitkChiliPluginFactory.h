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

#ifndef MITK_CHILIPLUGINFACTORY_H_INCLUDED
#define MITK_CHILIPLUGINFACTORY_H_INCLUDED

#include <itkObjectFactoryBase.h>

namespace mitk {

  class ChiliPluginFactory : public ObjectFactoryBase {
    
    mitkClassMacro(mitk::ChiliPluginFactory,itk::ObjectFactoryBase)

      /** Class methods used to interface with the registered factories. */
      virtual const char* GetITKSourceVersion() const;
    virtual const char* GetDescription() const;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /** Register one factory of this type  */
    static void RegisterOneFactory()
    {
      ChiliPluginFactory::Pointer chiliPluginFactory = ChiliPluginFactory::New();
      ObjectFactoryBase::RegisterFactory(chiliPluginFactory);
    }
    protected:
    ChiliPluginFactory();
    ~ChiliPluginFactory();

    private:
    ChiliPluginFactory(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented


  }
}

#endif
