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
#ifndef CONTOURMODELWRITERFACTORY_H_HEADER_INCLUDED
#define CONTOURMODELWRITERFACTORY_H_HEADER_INCLUDED

#include "SegmentationExports.h"
#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{

  class Segmentation_EXPORT ContourModelWriterFactory : public itk::ObjectFactoryBase
  {
  public:

    mitkClassMacro( mitk::ContourModelWriterFactory, itk::ObjectFactoryBase )

      /** Class methods used to interface with the registered factories. */
      virtual const char* GetITKSourceVersion(void) const;
    virtual const char* GetDescription(void) const;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /** Register one factory of this type  */
    static void RegisterOneFactory(void)
    {
      static bool IsRegistered = false;
      if ( !IsRegistered )
      {
        ContourModelWriterFactory::Pointer contourModelWriterFactory = ContourModelWriterFactory::New();
        ObjectFactoryBase::RegisterFactory( contourModelWriterFactory );
        IsRegistered = true;
      }
    }

  protected:
    ContourModelWriterFactory();
    ~ContourModelWriterFactory();

  private:
    ContourModelWriterFactory(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

  };
}
#endif //CONTOURMODELWRITERFACTORY_H_HEADER_INCLUDED
