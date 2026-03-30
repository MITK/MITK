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
   * Registers ContourModelGLMapper2D, ContourModelMapper3D and their set counterparts
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

    /** \brief Return supported file extensions for reading.
     * \return String of supported file extensions.
     */
    std::string GetFileExtensions() override;

    /** \brief Return the file extension map for reading.
     * \return Multimap of file extension to description.
     */
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    /** \brief Return supported file extensions for saving.
     * \return String of supported save file extensions.
     */
    std::string GetSaveFileExtensions() override;

    /** \brief Return the file extension map for saving.
     * \return Multimap of save file extension to description.
     */
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

    /** \deprecatedSince{2013_09} */
    DEPRECATED(void RegisterIOFactories());

  protected:
    ContourObjectFactory();
    ~ContourObjectFactory() override;
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

  private:
  };
}

#endif
