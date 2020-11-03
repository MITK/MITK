/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSerializeBaseData_h_included
#define mitkSerializeBaseData_h_included

#include "mitkSerializerMacros.h"
#include <MitkSceneSerializationBaseExports.h>

#include "mitkBaseData.h"
#include <itkObjectFactoryBase.h>

namespace mitk
{
  /**
    \brief Base class for objects that serialize BaseData types.

    The name of sub-classes must be deduced from the class name of the object that should be serialized.
    The serialization assumes that

    \verbatim
    If the class derived from BaseData is called GreenData
    Then the serializer for this class must be called GreenDataSerializer
    \endverbatim
  */
  class MITKSCENESERIALIZATIONBASE_EXPORT BaseDataSerializer : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BaseDataSerializer, itk::Object);

    itkSetStringMacro(FilenameHint);
    itkGetStringMacro(FilenameHint);

    itkSetStringMacro(WorkingDirectory);
    itkGetStringMacro(WorkingDirectory);

    itkSetConstObjectMacro(Data, BaseData);

    /**
      \brief Serializes given BaseData object.
      \return the filename of the newly created file.

      This should be overwritten by specific sub-classes.
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
