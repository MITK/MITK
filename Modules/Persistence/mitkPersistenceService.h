/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPersistenceService_h
#define mitkPersistenceService_h

#include "mitkIPersistenceService.h"
#include "mitkPropertyListsXmlFileReaderAndWriter.h"
#include "mitkSceneIO.h"
#include <MitkPersistenceExports.h>
#include <itkLightObject.h>

namespace mitk
{
  ///
  /// implementation of the IPersistenceService
  /// \see IPersistenceService
  class MITKPERSISTENCE_EXPORT PersistenceService : public itk::LightObject, public mitk::IPersistenceService
  {
  public:
    static std::string GetPersistencePropertyName();

    static std::string GetPersistencePropertyListName();

    static void LoadModule();
    static us::ModuleContext *GetModuleContext();

    PersistenceService();

    ~PersistenceService() override;

    std::string GetDefaultPersistenceFile() override;

    mitk::PropertyList::Pointer GetPropertyList(std::string &id, bool *existed = nullptr) override;

    bool RemovePropertyList(std::string &id) override;

    std::string GetPersistenceNodePropertyName() override;

    DataStorage::SetOfObjects::Pointer GetDataNodes(DataStorage *ds = nullptr) override;

    bool Save(const std::string &fileName = "", bool appendChanges = false) override;

    bool Load(const std::string &fileName = "", bool enforeReload = true) override;

    void SetAutoLoadAndSave(bool autoLoadAndSave) override;

    bool GetAutoLoadAndSave() override;

    void AddPropertyListReplacedObserver(PropertyListReplacedObserver *observer) override;

    void RemovePropertyListReplacedObserver(PropertyListReplacedObserver *observer) override;

    bool RestorePropertyListsFromPersistentDataNodes(const DataStorage *storage) override;

    void Clear();

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
