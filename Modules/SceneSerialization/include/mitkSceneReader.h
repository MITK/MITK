/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSceneReader_h
#define mitkSceneReader_h

#include <MitkSceneSerializationExports.h>

#include <itkObjectFactory.h>

#include <mitkDataStorage.h>

namespace tinyxml2
{
  class XMLDocument;
}

namespace mitk
{
  /**
   * \brief Reads a MITK scene from an XML document and populates a DataStorage.
   *
   * SceneReader is the dispatcher for versioned scene file reading. It reads the
   * version information from the XML document and delegates the actual loading
   * to a version-specific reader (e.g., SceneReaderV1) discovered via the ITK
   * object factory. This allows transparent handling of different scene file
   * format versions.
   *
   * \sa SceneIO, SceneReaderV1
   */
  class MITKSCENESERIALIZATION_EXPORT SceneReader : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SceneReader, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Loads a scene from a parsed XML document into the given DataStorage.
     *
     * Reads the file version from the XML document, instantiates the appropriate
     * versioned scene reader (e.g., SceneReaderV1), and delegates the loading to it.
     *
     * \param[in]  document         The parsed XML document (index.xml content).
     * \param[in]  workingDirectory The directory containing the serialized data files
     *                              referenced by the XML document.
     * \param[in]  storage          The DataStorage to populate with loaded DataNodes.
     *
     * \return True if the scene was loaded successfully, false if errors occurred
     *         (e.g., no reader found for the file version, or the reader reported errors).
     *
     * \pre \p storage must not be null.
     */
    virtual bool LoadScene(tinyxml2::XMLDocument &document, const std::string &workingDirectory, DataStorage *storage);
  };
}

#endif
