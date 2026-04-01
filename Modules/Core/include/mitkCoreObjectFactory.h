/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCoreObjectFactory_h
#define mitkCoreObjectFactory_h

#include <set>

#include <mitkCoreObjectFactoryBase.h>
#include <mitkFileWriterWithInformation.h>
#include <MitkCoreExports.h>
namespace mitk
{
  class Event;
  class LegacyFileReaderService;
  class LegacyFileWriterService;
  class LegacyImageWriterService;

  /**
   * \brief The central factory for creating mappers and setting default properties for MITK core data types.
   *
   * Implements the CoreObjectFactoryBase interface. Additional module-specific
   * factories can be registered via RegisterExtraFactory() to extend the set
   * of supported mappers and default properties.
   *
   * \ingroup DataManagement
   * \sa CoreObjectFactoryBase
   */
  class MITKCORE_EXPORT CoreObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(CoreObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(CoreObjectFactory);

    /**
     * \brief Create a mapper for the given data node and mapper slot.
     * \param node   The data node requiring a mapper.
     * \param slotId The mapper slot (2D or 3D).
     * \return A new mapper instance, or nullptr if no mapper is available.
     */
    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    /**
     * \brief Set default properties on the given data node.
     * \param node The data node to configure with default rendering properties.
     */
    void SetDefaultProperties(mitk::DataNode *node) override;

    /**
     * \brief Map a legacy event to an internal event ID.
     * \param event   The event to map.
     * \param eventID The internal event identifier.
     */
    virtual void MapEvent(const mitk::Event *event, const int eventID);

    /**
     * \brief Register an additional module-specific object factory.
     *
     * The extra factory's mappers and default properties become available
     * through this CoreObjectFactory.
     *
     * \param factory The factory to register.
     */
    virtual void RegisterExtraFactory(CoreObjectFactoryBase *factory);

    /**
     * \brief Unregister a previously registered extra factory.
     * \param factory The factory to remove.
     */
    virtual void UnRegisterExtraFactory(CoreObjectFactoryBase *factory);

    /**
     * \brief Return the singleton instance of the CoreObjectFactory.
     * \return The singleton pointer.
     */
    static Pointer GetInstance();

    /** \brief Destructor. */
    ~CoreObjectFactory() override;

    /**
     * \brief Get the supported (open) file extensions as a string for QFileDialog.
     * \return A string containing the supported file extensions.
     * \deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetFileExtensions() override);

    /**
     * \brief Get the defined (open) file extension map.
     * \return A multimap of file extension to description pairs.
     * \deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual MultimapType GetFileExtensionsMap() override);

    /**
     * \brief Get the supported (save) file extensions as a string for QFileDialog.
     * \return A string containing the save file extensions.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetSaveFileExtensions() override);

    /**
     * \brief Get the defined (save) file extension map.
     * \return A multimap of save file extension to description pairs.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    MultimapType GetSaveFileExtensionsMap() override;

    /**
     * \brief Get the list of registered file writers.
     * \return A list of FileWriterWithInformation pointers.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry
     */
    DEPRECATED(virtual FileWriterList GetFileWriters());

    /**
     * \brief Get a human-readable description for the given file extension.
     * \param extension The file extension (e.g. "dcm").
     * \return A description string for the extension.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(std::string GetDescriptionForExtension(const std::string &extension));

  protected:
    /** \brief Constructor. Registers legacy readers and writers. */
    CoreObjectFactory();

    /**
     * \brief Merge the input map into the fileExtensionsMap, removing duplicates.
     *
     * \param fileExtensionsMap The existing map to be extended, containing pairs like
     *        ("*.dcm", "DICOM files").
     * \param inputMap The input map with additional extensions from extra factories.
     * \deprecatedSince{2014_10}
     */
    void MergeFileExtensions(MultimapType &fileExtensionsMap, MultimapType inputMap);

    /**
     * \brief Initialize the file extension entries for open and save.
     * \deprecatedSince{2014_10}
     */
    void CreateFileExtensionsMap();

    /**
     * \brief Create save file extension entries.
     * \deprecatedSince{2014_10}
     */
    DEPRECATED(void CreateSaveFileExtensions());

    typedef std::set<mitk::CoreObjectFactoryBase::Pointer> ExtraFactoriesContainer;

    ExtraFactoriesContainer m_ExtraFactories;
    FileWriterList m_FileWriters;
    std::string m_FileExtensions;
    MultimapType m_FileExtensionsMap;
    std::string m_SaveFileExtensions;
    MultimapType m_SaveFileExtensionsMap;

  private:
    void RegisterLegacyReaders(mitk::CoreObjectFactoryBase *factory);
    void RegisterLegacyWriters(mitk::CoreObjectFactoryBase *factory);

    void UnRegisterLegacyReaders(mitk::CoreObjectFactoryBase *factory);
    void UnRegisterLegacyWriters(mitk::CoreObjectFactoryBase *factory);

    std::map<mitk::CoreObjectFactoryBase *, std::list<mitk::LegacyFileReaderService *>> m_LegacyReaders;
    std::map<mitk::CoreObjectFactoryBase *, std::list<mitk::LegacyFileWriterService *>> m_LegacyWriters;
  };

} // namespace mitk

#endif
