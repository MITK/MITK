/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCoreObjectFactoryBase_h
#define mitkCoreObjectFactoryBase_h

// the mitkLog.h header is necessary for CMake test drivers.
// Since the EXTRA_INCLUDE parameter of CREATE_TEST_SOURCELIST only
// allows one extra include file, we specify mitkLog.h here so it will
// be available to all classes implementing this interface.
#include <mitkLog.h>

#include <mitkFileWriterWithInformation.h>
#include <mitkMapper.h>
#include <MitkCoreExports.h>
#include <itkObjectFactoryBase.h>
#include <itkVersion.h>

namespace mitk
{
  class DataNode;

  /**
   * \brief Abstract base class for factories that create mappers and set default properties.
   *
   * This interface can be implemented by factories that add new mapper classes
   * or extend the data tree deserialization mechanism. Module-specific factories
   * deriving from this class can be registered with CoreObjectFactory.
   *
   * \ingroup Algorithms
   * \sa CoreObjectFactory Mapper DataNode
   */
  class MITKCORE_EXPORT CoreObjectFactoryBase : public itk::Object
  {
  public:
    /** \brief List of file writers with associated information. */
    typedef std::list<mitk::FileWriterWithInformation::Pointer> FileWriterList;
    /** \brief Multimap of file extension to description string. */
    typedef std::multimap<std::string, std::string> MultimapType;

    mitkClassMacroItkParent(CoreObjectFactoryBase, itk::Object);

    /**
     * \brief Create a mapper for the given data node and mapper slot.
     * \param node   The data node to create a mapper for.
     * \param slotId The mapper slot (2D or 3D).
     * \return A new mapper, or nullptr if not supported.
     */
    virtual Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) = 0;

    /**
     * \brief Set default rendering properties on the given data node.
     * \param node The data node to configure.
     */
    virtual void SetDefaultProperties(mitk::DataNode *node) = 0;

    /**
     * \brief Get supported (open) file extensions as a string.
     * \return A filter string suitable for QFileDialog.
     * \deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    virtual std::string GetFileExtensions() = 0;

    /**
     * \brief Get the (open) file extension map.
     * \return A multimap of extension to description.
     * \deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    virtual MultimapType GetFileExtensionsMap() = 0;

    /**
     * \brief Get supported (save) file extensions as a string.
     * \return A filter string suitable for QFileDialog.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    virtual std::string GetSaveFileExtensions() = 0;

    /**
     * \brief Get the (save) file extension map.
     * \return A multimap of extension to description.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    virtual MultimapType GetSaveFileExtensionsMap() = 0;

    /**
     * \brief Get the ITK source version string.
     * \return The ITK source version used to build MITK.
     */
    virtual const char *GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }

    /**
     * \brief Get a human-readable description of this factory.
     * \return The description string.
     */
    virtual const char *GetDescription() const { return "Core Object Factory"; }

    /**
     * \brief Get the list of registered file writers.
     * \return A list of FileWriterWithInformation pointers.
     * \deprecatedSince{2014_10} See mitk::FileWriterRegistry
     */
    FileWriterList GetFileWriters() { return m_FileWriters; }

  protected:
    /**
     * \brief Create a QFileDialog filter string from a file extension map.
     * \param fileExtensionsMap Input map, e.g. ("*.dcm", "DICOM files").
     * \param fileExtensions    Output string suitable for QFileDialog.
     * \deprecatedSince{2014_10}
     */
    static void CreateFileExtensions(MultimapType fileExtensionsMap, std::string &fileExtensions);

    FileWriterList m_FileWriters;  ///< Registered file writers.

    friend class CoreObjectFactory;
  };
}
#endif
