/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkSerializeBaseData_h_included
#define mitkSerializeBaseData_h_included

#include "SceneSerializationExports.h"
#include "mitkSerializerMacros.h"

#include <itkObjectFactoryBase.h>
#include "mitkBaseData.h"

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
class SceneSerialization_EXPORT BaseDataSerializer : public itk::Object
{
  public:
    
    mitkClassMacro( BaseDataSerializer, itk::Object );

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
    virtual ~BaseDataSerializer();

    std::string GetUniqueFilenameInWorkingDirectory();
    
    std::string m_FilenameHint;
    std::string m_WorkingDirectory;
    BaseData::ConstPointer m_Data;
};

} // namespace

#endif

