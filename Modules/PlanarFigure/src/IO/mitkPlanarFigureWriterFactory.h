/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED
#define PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED

#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{
  class PlanarFigureWriterFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacroItkParent(mitk::PlanarFigureWriterFactory, itk::ObjectFactoryBase);

      /** Class methods used to interface with the registered factories. */
      const char *GetITKSourceVersion(void) const override;
    const char *GetDescription(void) const override;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

      /**
       * Register one factory of this type
       * \deprecatedSince{2013_09}
       */
      DEPRECATED(static void RegisterOneFactory(void));

    /**
     * UnRegister one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void UnRegisterOneFactory(void));

  protected:
    PlanarFigureWriterFactory();
    ~PlanarFigureWriterFactory() override;

  private:
    PlanarFigureWriterFactory(const Self &); // purposely not implemented
    void operator=(const Self &);            // purposely not implemented

    static itk::ObjectFactoryBase::Pointer GetInstance();
  };

} // end namespace mitk

#endif // PLANARFIGURE_WRITERFACTORY_H_HEADER_INCLUDED
