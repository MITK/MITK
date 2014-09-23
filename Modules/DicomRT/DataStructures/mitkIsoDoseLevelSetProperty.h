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


#ifndef _MITK_DOSE_ISO_LEVEL_SET_PROPERTY_H_
#define _MITK_DOSE_ISO_LEVEL_SET_PROPERTY_H_

#include "mitkBaseProperty.h"
#include "mitkIsoDoseLevelCollections.h"
#include "MitkDicomRTExports.h"

namespace mitk {

/**
\brief Property class for dose iso level sets.
*/
class MitkDicomRT_EXPORT IsoDoseLevelSetProperty : public BaseProperty
{

protected:
    IsoDoseLevelSet::Pointer m_IsoLevelSet;

    IsoDoseLevelSetProperty();

    IsoDoseLevelSetProperty(const IsoDoseLevelSetProperty& other);

    IsoDoseLevelSetProperty(IsoDoseLevelSet* levelSet);

public:
    mitkClassMacro(IsoDoseLevelSetProperty, BaseProperty);

    itkNewMacro(IsoDoseLevelSetProperty);
    mitkNewMacro1Param(IsoDoseLevelSetProperty, IsoDoseLevelSet*);

    typedef IsoDoseLevelSet ValueType;

    virtual ~IsoDoseLevelSetProperty();

    const IsoDoseLevelSet * GetIsoDoseLevelSet() const;
    const IsoDoseLevelSet * GetValue() const;
    IsoDoseLevelSet * GetIsoDoseLevelSet();
    IsoDoseLevelSet * GetValue();


    void SetIsoDoseLevelSet(IsoDoseLevelSet* levelSet);
    void SetValue(IsoDoseLevelSet* levelSet);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

private:

    // purposely not implemented
    IsoDoseLevelSetProperty& operator=(const IsoDoseLevelSetProperty&);

    itk::LightObject::Pointer InternalClone() const;

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

};

} // namespace mitk


#endif /* _MITK_DOSE_ISO_LEVEL_SET_PROPERTY_H_ */
