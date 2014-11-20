/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef COREOBJECTFACTORYBASE_H_INCLUDED
#define COREOBJECTFACTORYBASE_H_INCLUDED

// the mbilog header is necessary for CMake test drivers.
// Since the EXTRA_INCLUDE parameter of CREATE_TEST_SOURCELIST only
// allows one extra include file, we specify mitkLog.h here so it will
// be available to all classes implementing this interface.
#include "mitkLog.h"

#include <MitkCoreExports.h>
#include "mitkMapper.h"
#include <itkObjectFactoryBase.h>
#include <itkVersion.h>
#include "mitkFileWriterWithInformation.h"

namespace mitk {

class DataNode;

//## @brief base-class for factories of certain mitk objects.
//## @ingroup Algorithms
//## This interface can be implemented by factories which add new mapper classes or extend the
//## data tree deserialization mechanism.

class MITK_CORE_EXPORT CoreObjectFactoryBase : public itk::Object
{
  public:

    typedef std::list<mitk::FileWriterWithInformation::Pointer> FileWriterList;
    typedef std::multimap<std::string, std::string> MultimapType;

    mitkClassMacro(CoreObjectFactoryBase,itk::Object)

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) = 0;
    virtual void SetDefaultProperties(mitk::DataNode* node) = 0;

    /**
     * @deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    virtual const char* GetFileExtensions() = 0;

    /**
     * @deprecatedSince{2014_10} See mitk::FileReaderRegistry and QmitkIOUtil
     */
    virtual MultimapType GetFileExtensionsMap() = 0;

    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    virtual const char* GetSaveFileExtensions() = 0;

    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    virtual MultimapType GetSaveFileExtensionsMap() = 0;

    virtual const char* GetITKSourceVersion() const
    {
      return ITK_SOURCE_VERSION;
    }
    virtual const char* GetDescription() const
    {
      return "Core Object Factory";
    }

    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry
     */
    FileWriterList GetFileWriters()
    {
      return m_FileWriters;
    }

  protected:

    /**
     * @brief create a string from a map that contains the file extensions
     * @param fileExtensionsMap input map with the file extensions, e.g. ("*.dcm", "DICOM files")("*.dc3", "DICOM files")
     * @param fileExtensions the converted output string, suitable for the QT QFileDialog widget
     *                       e.g. "all (*.dcm *.DCM *.dc3 ... *.vti *.hdr *.nrrd *.nhdr );;Q-Ball Images (*.hqbi *qbi)"
     *
     * @deprecatedSince{2014_10}
     */
    static void CreateFileExtensions(MultimapType fileExtensionsMap, std::string& fileExtensions);

    FileWriterList m_FileWriters;

    friend class CoreObjectFactory;
};
}
#endif
