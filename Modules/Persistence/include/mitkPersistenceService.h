/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPersistenceService_h
#define mitkPersistenceService_h

#include <mitkIPersistenceService.h>
#include <mitkPropertyListsXmlFileReaderAndWriter.h>
#include <mitkSceneIO.h>
#include <MitkPersistenceExports.h>
#include <itkLightObject.h>

namespace mitk
{
  /**
   * \brief Concrete implementation of the IPersistenceService interface.
   *
   * This service manages a collection of named PropertyList objects that can be
   * persisted to and restored from files. It supports both XML-based storage
   * (via PropertyListsXmlFileReaderAndWriter) and MITK scene file storage
   * (via SceneIO). The service is registered as a micro service and can be
   * retrieved via the module context.
   *
   * On first use, the service lazily initializes itself by loading the default
   * persistence file. If auto-load-and-save is enabled, property lists are
   * automatically saved when Unitialize() is called (typically at application shutdown).
   *
   * \sa IPersistenceService
   * \sa IPersistable
   * \sa PropertyListsXmlFileReaderAndWriter
   */
  class MITKPERSISTENCE_EXPORT PersistenceService : public itk::LightObject, public mitk::IPersistenceService
  {
  public:
    /**
     * \brief Get the property name used to mark DataNodes as persistence nodes.
     *
     * \return The string "PersistenceNode".
     */
    static std::string GetPersistencePropertyName();

    /**
     * \brief Get the property list name used for internal persistence service settings.
     *
     * \return The string "PersistenceService".
     */
    static std::string GetPersistencePropertyListName();

    /**
     * \brief Trigger loading of the persistence module.
     *
     * This is a no-op beyond logging; it exists so that the module is loaded
     * by the micro services framework on demand.
     */
    static void LoadModule();

    /**
     * \brief Get the micro services module context for this module.
     *
     * \return Pointer to the us::ModuleContext.
     */
    static us::ModuleContext *GetModuleContext();

    PersistenceService();

    ~PersistenceService() override;

    /**
     * \brief Get the default file path used for persisting property lists.
     *
     * Returns "PersistentData.xml" located in the module's persistent data directory,
     * or just "PersistentData.xml" in the working directory if no data directory is available.
     *
     * \return The absolute or relative path to the default persistence file.
     */
    std::string GetDefaultPersistenceFile() override;

    /**
     * \brief Retrieve or create a PropertyList identified by the given id.
     *
     * If the id is empty, a new UUID is generated and assigned to the id parameter.
     * If a PropertyList with the given id already exists, it is returned; otherwise a
     * new empty PropertyList is created and stored.
     *
     * \param[in,out] id The identifier string. If empty, a UUID will be generated and written back.
     * \param[out] existed Optional output flag; set to true if the PropertyList already existed, false if newly created.
     * \return A valid PropertyList associated with the given id.
     */
    mitk::PropertyList::Pointer GetPropertyList(std::string &id, bool *existed = nullptr) override;

    /**
     * \brief Remove the PropertyList identified by the given id.
     *
     * \param[in] id The identifier of the PropertyList to remove.
     * \return True if a PropertyList with the given id existed and was removed, false otherwise.
     */
    bool RemovePropertyList(std::string &id) override;

    /**
     * \brief Get the name of the boolean property used to tag persistence DataNodes.
     *
     * \return The persistence property name string.
     * \sa GetPersistencePropertyName()
     */
    std::string GetPersistenceNodePropertyName() override;

    /**
     * \brief Create DataNodes from all stored PropertyLists and optionally add them to a DataStorage.
     *
     * Each PropertyList is cloned into a new DataNode. The DataNode's name is set to the
     * PropertyList's id, and a boolean property (named per GetPersistencePropertyName()) is
     * set to true.
     *
     * \param[in,out] ds Optional DataStorage to which created DataNodes are added. May be nullptr.
     * \return A set of newly created DataNodes containing the persisted property lists.
     */
    DataStorage::SetOfObjects::Pointer GetDataNodes(DataStorage *ds = nullptr) override;

    /**
     * \brief Save all property lists to a file.
     *
     * If the file has an ".xml" extension, property lists are written as XML via
     * PropertyListsXmlFileReaderAndWriter. Otherwise, the MITK SceneIO format is used.
     * Directories are created as needed.
     *
     * \param[in] fileName Path to the output file. If empty, the default persistence file is used.
     * \param[in] appendChanges If true, existing data in the file is loaded first and merged before saving.
     * \return True on success, false if an error occurred (e.g., cannot write to file).
     */
    bool Save(const std::string &fileName = "", bool appendChanges = false) override;

    /**
     * \brief Load property lists from a file.
     *
     * If the file has an ".xml" extension, property lists are read as XML via
     * PropertyListsXmlFileReaderAndWriter. Otherwise, the MITK SceneIO format is used.
     * If enforceReload is false, the file is only reloaded when its modification time
     * has changed since the last load.
     *
     * \param[in] fileName Path to the input file. If empty, the default persistence file is used.
     * \param[in] enforeReload If true, always reload; if false, skip reloading unchanged files.
     * \return True on success, false if an error occurred (e.g., file not found or parse error).
     * \note Existing PropertyLists with matching ids will be overwritten.
     * \sa AddPropertyListReplacedObserver()
     */
    bool Load(const std::string &fileName = "", bool enforeReload = true) override;

    /**
     * \brief Enable or disable automatic loading and saving of property lists.
     *
     * When enabled, property lists are automatically loaded at initialization and
     * saved at shutdown (via Unitialize()).
     *
     * \param[in] autoLoadAndSave True to enable auto-load-and-save, false to disable.
     */
    void SetAutoLoadAndSave(bool autoLoadAndSave) override;

    /**
     * \brief Query whether automatic loading and saving is enabled.
     *
     * \return True if auto-load-and-save is enabled, false otherwise.
     */
    bool GetAutoLoadAndSave() override;

    /**
     * \brief Register an observer to be notified when a PropertyList is replaced during Load().
     *
     * \param[in] observer Pointer to the observer to add. Must not be nullptr.
     * \sa RemovePropertyListReplacedObserver()
     */
    void AddPropertyListReplacedObserver(PropertyListReplacedObserver *observer) override;

    /**
     * \brief Unregister a previously added PropertyListReplacedObserver.
     *
     * \param[in] observer Pointer to the observer to remove.
     * \sa AddPropertyListReplacedObserver()
     */
    void RemovePropertyListReplacedObserver(PropertyListReplacedObserver *observer) override;

    /**
     * \brief Restore property lists from persistence-tagged DataNodes in a DataStorage.
     *
     * Scans all DataNodes in the given storage for ones marked with the persistence
     * property. For each such node, its properties are cloned into the corresponding
     * PropertyList managed by this service. Registered PropertyListReplacedObservers
     * are notified before and after replacement.
     *
     * \param[in] storage The DataStorage to scan for persistence nodes. Must not be nullptr.
     * \return True if at least one persistence node was found and restored, false otherwise.
     */
    bool RestorePropertyListsFromPersistentDataNodes(const DataStorage *storage) override;

    /**
     * \brief Clear all stored property lists and file modification time records.
     */
    void Clear();

    /**
     * \brief Shut down the persistence service.
     *
     * If auto-load-and-save is enabled, all property lists are saved to the
     * default file before shutdown.
     */
    void Unitialize();

  private:
    void ClonePropertyList(mitk::PropertyList *from, mitk::PropertyList *to) const;
    void Initialize();
    std::map<std::string, mitk::PropertyList::Pointer> m_PropertyLists;
    bool m_AutoLoadAndSave;
    std::set<PropertyListReplacedObserver *> m_PropertyListReplacedObserver;
    SceneIO::Pointer m_SceneIO;
    PropertyListsXmlFileReaderAndWriter::Pointer m_PropertyListsXmlFileReaderAndWriter;
    std::map<std::string, long int> m_FileNamesToModifiedTimes;
    bool m_Initialized;
    bool m_InInitialized;
  };
}
#endif
