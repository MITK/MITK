/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseDataSerializer_h
#define mitkBaseDataSerializer_h

#include <mitkSerializerMacros.h>
#include <MitkSceneSerializationBaseExports.h>

#include <mitkBaseData.h>
#include <itkObjectFactoryBase.h>

namespace mitk
{
  /**
   * \brief Base class for objects that serialize BaseData types to files.
   *
   * Provides the interface for serializing mitk::BaseData-derived objects into
   * files on disk. Sub-classes must implement the actual serialization logic.
   *
   * The naming convention for sub-classes must follow this pattern:
   * \verbatim
   * If the class derived from BaseData is called GreenData
   * Then the serializer for this class must be called GreenDataSerializer
   * \endverbatim
   *
   * Sub-classes are discovered at runtime through the ITK object factory mechanism.
   * Use the MITK_REGISTER_SERIALIZER macro to register a new serializer.
   *
   * \sa BasePropertySerializer, PropertyListSerializer, SceneIO, MITK_REGISTER_SERIALIZER
   */
  class MITKSCENESERIALIZATIONBASE_EXPORT BaseDataSerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BaseDataSerializer, itk::Object);

    /** \brief Sets a hint for the output filename (without path or extension). */
    itkSetStringMacro(FilenameHint);
    /** \brief Returns the current filename hint. */
    itkGetStringMacro(FilenameHint);

    /** \brief Sets the working directory where serialized files will be stored. */
    itkSetStringMacro(WorkingDirectory);
    /** \brief Returns the current working directory for serialized files. */
    itkGetStringMacro(WorkingDirectory);

    /**
     * \brief Sets the BaseData object to be serialized.
     * \param[in] _arg Pointer to the BaseData object.
     */
    itkSetConstObjectMacro(Data, BaseData);

    /**
     * \brief Serializes the previously set BaseData object to a file.
     *
     * The base implementation logs the request and returns an empty string.
     * Sub-classes must override this method to perform the actual serialization,
     * writing data to a file in the working directory.
     *
     * \return The filename (without path) of the newly created file, or an
     *         empty string if serialization failed or is not implemented.
     *
     * \pre The Data object must be set via SetData().
     * \pre The WorkingDirectory must be set via SetWorkingDirectory().
     */
    virtual std::string Serialize();

  protected:
    BaseDataSerializer();
    ~BaseDataSerializer() override;

    std::string GetUniqueFilenameInWorkingDirectory();

    std::string m_FilenameHint;
    std::string m_WorkingDirectory;
    BaseData::ConstPointer m_Data;
  };

} // namespace

#endif
