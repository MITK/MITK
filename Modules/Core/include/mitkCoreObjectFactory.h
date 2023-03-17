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

#include "mitkCoreObjectFactoryBase.h"
#include "mitkFileWriterWithInformation.h"
#include <MitkCoreExports.h>
namespace mitk
{
  class Event;
  class LegacyFileReaderService;
  class LegacyFileWriterService;
  class LegacyImageWriterService;

  class MITKCORE_EXPORT CoreObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(CoreObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(CoreObjectFactory);

      Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode *node) override;

    virtual void MapEvent(const mitk::Event *event, const int eventID);

    virtual void RegisterExtraFactory(CoreObjectFactoryBase *factory);
    virtual void UnRegisterExtraFactory(CoreObjectFactoryBase *factory);

    static Pointer GetInstance();

    ~CoreObjectFactory() override;

    /**
     * @brief This method gets the supported (open) file extensions as string.
     *
     * This string can then used by the Qt QFileDialog widget.
     *
     * @return The c-string that contains the file extensions
     * @deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetFileExtensions() override);

    /**
     * @brief get the defined (open) file extension map
     *
     * @return the defined (open) file extension map
     * @deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual MultimapType GetFileExtensionsMap() override);

    /**
     * @brief This method gets the supported (save) file extensions as string.
     *
     * This string can then used by the Qt QFileDialog widget.
     *
     * @return The c-string that contains the (save) file extensions
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetSaveFileExtensions() override);

    /**
     * @brief get the defined (save) file extension map
     *
     * @return the defined (save) file extension map
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    MultimapType GetSaveFileExtensionsMap() override;

    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry
     */
    DEPRECATED(virtual FileWriterList GetFileWriters());

    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(std::string GetDescriptionForExtension(const std::string &extension));

  protected:
    CoreObjectFactory();

    /**
     * @brief Merge the input map into the fileExtensionsMap. Duplicate entries are removed
     *
     * @param fileExtensionsMap the existing map, it contains value pairs like
     *        ("*.dcm", "DICOM files"),("*.dc3", "DICOM files").
     *        This map is extended/merged with the values from the input map.
     * @param inputMap the input map, it contains value pairs like ("*.dcm",
     *        "DICOM files"),("*.dc3", "DICOM files") returned by the extra factories.
     * @deprecatedSince{2014_10}
     */
    void MergeFileExtensions(MultimapType &fileExtensionsMap, MultimapType inputMap);

    /**
     * @brief initialize the file extension entries for open and save
     * @deprecatedSince{2014_10}
     */
    void CreateFileExtensionsMap();

    /**
     * @deprecatedSince{2014_10}
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
