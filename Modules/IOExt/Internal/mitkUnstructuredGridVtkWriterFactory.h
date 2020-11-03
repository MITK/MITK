/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef UNSTRUCTURED_GRID_WRITERFACTORY_H_HEADER_INCLUDED
#define UNSTRUCTURED_GRID_WRITERFACTORY_H_HEADER_INCLUDED

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  class UnstructuredGridVtkWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacroItkParent(mitk::UnstructuredGridVtkWriterFactory, itk::ObjectFactoryBase);

      /** Class methods used to interface with the registered factories. */
      const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /**
     * Register one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void))
    {
      static bool IsRegistered = false;
      if (!IsRegistered)
      {
        UnstructuredGridVtkWriterFactory::Pointer ugVtkWriterFactory = UnstructuredGridVtkWriterFactory::New();
        ObjectFactoryBase::RegisterFactory(ugVtkWriterFactory);
        IsRegistered = true;
      }
    }

  protected:
    UnstructuredGridVtkWriterFactory();
    ~UnstructuredGridVtkWriterFactory() override;

  private:
    UnstructuredGridVtkWriterFactory(const Self &); // purposely not implemented
    void operator=(const Self &);                   // purposely not implemented
  };

} // end namespace mitk

#endif // UNSTRUCTURED_GRID_WRITERFACTORY_H_HEADER_INCLUDED
