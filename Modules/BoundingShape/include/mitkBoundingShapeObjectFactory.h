/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeObjectFactory_h
#define mitkBoundingShapeObjectFactory_h

#include <MitkBoundingShapeExports.h>
#include <mitkCoreObjectFactoryBase.h>

namespace mitk
{
  /** \brief Object factory that registers mappers and default properties for bounding shape data.
   *
   * Creates BoundingShapeVtkMapper2D and BoundingShapeVtkMapper3D instances for DataNodes
   * containing GeometryData, and sets default rendering properties for bounding shape visualization.
   *
   * \sa BoundingShapeVtkMapper2D, BoundingShapeVtkMapper3D, CoreObjectFactoryBase
   */
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(BoundingShapeObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Create a mapper for bounding shape rendering.
     *
     * \param[in] node   The data node to create a mapper for.
     * \param[in] slotId The mapper slot (2D or 3D).
     * \return A mapper instance, or \c nullptr if the node data is not GeometryData.
     */
    Mapper::Pointer CreateMapper(DataNode *node, MapperSlotId slotId) override;

    /** \brief Set default rendering properties for bounding shape visualization on the given node.
     *
     * \param[in] node The data node to configure.
     */
    void SetDefaultProperties(DataNode *node) override;

    /** \brief Get supported file extensions (empty for bounding shapes).
     * \return An empty string.
     */
    std::string GetFileExtensions() override;

    /** \brief Get the file extensions map.
     * \return An empty multimap.
     */
    CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    /** \brief Get supported save file extensions (empty for bounding shapes).
     * \return An empty string.
     */
    std::string GetSaveFileExtensions() override;

    /** \brief Get the save file extensions map.
     * \return An empty multimap.
     */
    CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

    /** \brief Get a human-readable description of this factory.
     * \return A description string.
     */
    const char *GetDescription() const override;

  protected:
    BoundingShapeObjectFactory();
    ~BoundingShapeObjectFactory() override;

    void CreateFileExtensionsMap();

  private:
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };

  /** \brief Register the BoundingShapeObjectFactory with the CoreObjectFactory.
   *
   * Call this function once during application startup to enable bounding shape
   * rendering. Typically invoked from module activators.
   */
  MITKBOUNDINGSHAPE_EXPORT void RegisterBoundingShapeObjectFactory();
}

#endif
