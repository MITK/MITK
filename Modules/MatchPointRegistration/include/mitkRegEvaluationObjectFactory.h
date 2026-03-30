/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRegEvaluationObjectFactory_h
#define mitkRegEvaluationObjectFactory_h

#include <mitkCoreObjectFactory.h>
#include <MitkMatchPointRegistrationExports.h>

namespace mitk {

  /**
   * \brief Factory that registers the mapper needed for rendering RegEvaluationObject instances in MITK.
   *
   * This CoreObjectFactory creates the RegEvaluationMapper2D for 2D slice rendering and
   * sets default properties on data nodes containing RegEvaluationObject data.
   *
   * \sa mitk::RegEvaluationObject, mitk::RegEvaluationMapper2D
   */
  class RegEvaluationObjectFactory : public mitk::CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(RegEvaluationObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(RegEvaluationObjectFactory);

    ~RegEvaluationObjectFactory() override;

    /**
     * \brief Sets the default rendering properties for a RegEvaluationObject node.
     * \param[in] node Pointer to the DataNode to configure.
     */
    void SetDefaultProperties(mitk::DataNode* node) override;

    /**
     * \brief Returns supported file extensions for reading (empty for this factory).
     * \return File extension string.
     */
    std::string GetFileExtensions() override;

    /**
     * \brief Returns supported file extensions as a multimap (empty for this factory).
     * \return File extension multimap.
     */
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    /**
     * \brief Returns supported file extensions for saving (empty for this factory).
     * \return Save file extension string.
     */
    std::string GetSaveFileExtensions() override;

    /**
     * \brief Returns supported save file extensions as a multimap (empty for this factory).
     * \return Save file extension multimap.
     */
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

    /**
     * \brief Creates the appropriate mapper for a RegEvaluationObject node.
     *
     * \param[in] node Pointer to the DataNode.
     * \param[in] slotId The mapper slot (2D or 3D).
     * \return Smart pointer to the created mapper, or nullptr if the slot is not supported.
     */
    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

    /**
     * \brief Registers any IO factories needed by this object factory.
     */
    void RegisterIOFactories();

  protected:
    std::string m_FileExtensions;
    RegEvaluationObjectFactory();
  };

}

#endif
