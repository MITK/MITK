/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKSMARTPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791
#define MITKSMARTPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791

#include <MitkExports.h>
#include "mitkBaseProperty.h"
#include "mitkUIDGenerator.h"

#include<map>
#include<list>
#include<string>

namespace mitk {

//##Documentation
//## @brief Property containing a smart-pointer
//## @ingroup DataManagement
class MITK_CORE_EXPORT SmartPointerProperty : public BaseProperty
{
  public:
    mitkClassMacro(SmartPointerProperty, BaseProperty);

    itkNewMacro(SmartPointerProperty);
    mitkNewMacro1Param(SmartPointerProperty, itk::Object*);

    typedef itk::Object::Pointer ValueType;

    itk::Object::Pointer GetSmartPointer() const;
    ValueType GetValue() const;

    void SetSmartPointer(itk::Object*);
    void SetValue(const ValueType&);

    /// mainly for XML output
    virtual std::string GetValueAsString() const;
    
    static void PostProcessXMLReading();

    /// Return the number of SmartPointerProperties that reference the object given as parameter
    static unsigned int GetReferenceCountFor(itk::Object*);
    static std::string GetReferenceUIDFor(itk::Object*);
    static void RegisterPointerTarget(itk::Object*, const std::string uid);

    using BaseProperty::operator=;

  protected:
    
    SmartPointerProperty(itk::Object* = NULL);

    itk::Object::Pointer m_SmartPointer;

  private:

    // purposely not implemented
    SmartPointerProperty(const SmartPointerProperty&);
    SmartPointerProperty& operator=(const SmartPointerProperty&);

    virtual bool IsEqual(const BaseProperty&) const;
    virtual bool Assign(const BaseProperty&);

    typedef std::map<itk::Object*, unsigned int>             ReferenceCountMapType;
    typedef std::map<itk::Object*, std::string>              ReferencesUIDMapType;
    typedef std::map<SmartPointerProperty*, std::string>     ReadInSmartPointersMapType;
    typedef std::map<std::string, itk::Object*>              ReadInTargetsMapType;

    /// for each itk::Object* count how many SmartPointerProperties point to it
    static ReferenceCountMapType          m_ReferenceCount;
    static ReferencesUIDMapType           m_ReferencesUID;
    static ReadInSmartPointersMapType     m_ReadInInstances;
    static ReadInTargetsMapType           m_ReadInTargets;

    /// to generate unique IDs for the objects pointed at (during XML writing)
    static UIDGenerator m_UIDGenerator;
};

} // namespace mitk



#endif /* MITKSMARTPOINTERPROPERTY_H_HEADER_INCLUDED_C126B791 */
