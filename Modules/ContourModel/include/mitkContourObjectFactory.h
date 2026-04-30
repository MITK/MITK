/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourObjectFactory_h
#define mitkContourObjectFactory_h

#include <mitkCoreObjectFactoryBase.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  /** \brief Object factory that registers mappers and I/O for contour model types.
   *
   * Registers ContourModelMapper2D, ContourModelMapper3D and their set counterparts
   * with the MITK core object factory so that ContourModel and ContourModelSet data
   * can be rendered and serialized.
   *
   * \sa ContourModel, ContourModelSet, CoreObjectFactoryBase
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(ContourObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)

    /** \brief Create the appropriate mapper for contour data at the given slot.
     * \param[in] node The data node containing contour data.
     * \param[in] slotId The mapper slot (2D or 3D).
     * \return Smart pointer to the created Mapper, or nullptr if not applicable.
     */
    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    /** \brief Set default rendering properties for contour data nodes.
     * \param[in] node The data node to configure.
     */
    void SetDefaultProperties(mitk::DataNode *node) override;

  protected:
    ContourObjectFactory();
    ~ContourObjectFactory() override;
  };
}

#endif
