/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkMAPRegistrationWrapperObjectFactory_h
#define mitkMAPRegistrationWrapperObjectFactory_h

#include <mitkCoreObjectFactory.h>
#include <MitkMatchPointRegistrationExports.h>

namespace mitk {

/**
 * \brief Factory that registers mappers and default properties for MAPRegistrationWrapper objects in MITK.
 *
 * This CoreObjectFactory registers the 2D and 3D visualization mappers for
 * MAPRegistrationWrapper data and sets default rendering properties on data nodes.
 *
 * \sa mitk::MAPRegistrationWrapper, mitk::MITKRegistrationWrapperMapper2D, mitk::MITKRegistrationWrapperMapper3D
 */
class MAPRegistrationWrapperObjectFactory : public mitk::CoreObjectFactoryBase
{
  public:
    mitkClassMacro(MAPRegistrationWrapperObjectFactory,CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    ~MAPRegistrationWrapperObjectFactory() override;

    /**
     * \brief Sets the default rendering properties for a MAPRegistrationWrapper node.
     * \param[in] node Pointer to the DataNode to configure.
     */
    void SetDefaultProperties(mitk::DataNode* node) override;

    /**
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetFileExtensions());
    /**
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap());
    /**
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetSaveFileExtensions());
    /**
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap());

    /**
     * \brief Creates a mapper for the given node and mapper slot.
     *
     * Returns the appropriate 2D or 3D registration wrapper mapper depending on the slot ID.
     *
     * \param[in] node Pointer to the DataNode to create a mapper for.
     * \param[in] slotId The mapper slot (2D or 3D).
     * \return A smart pointer to the created mapper, or nullptr if no suitable mapper exists.
     */
    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

  protected:
    MAPRegistrationWrapperObjectFactory();
};

}

#endif
