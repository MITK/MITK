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


#ifndef _MITK_DOSE_ISO_LEVEL_VECTOR_PROPERTY_H_
#define _MITK_DOSE_ISO_LEVEL_VECTOR_PROPERTY_H_

#include "mitkBaseProperty.h"
#include "mitkIsoDoseLevelCollections.h"
#include "MitkDicomRTExports.h"

namespace mitk {

/**
\brief Property class for dose iso level vector.
*/
class MitkDicomRT_EXPORT IsoDoseLevelVectorProperty : public BaseProperty
{

protected:
    IsoDoseLevelVector::Pointer m_IsoLevelVector;

    IsoDoseLevelVectorProperty();

    IsoDoseLevelVectorProperty(const IsoDoseLevelVectorProperty& other);

    IsoDoseLevelVectorProperty(IsoDoseLevelVector* levelVector);

public:
    mitkClassMacro(IsoDoseLevelVectorProperty, BaseProperty);

    itkNewMacro(IsoDoseLevelVectorProperty);
    mitkNewMacro1Param(IsoDoseLevelVectorProperty, IsoDoseLevelVector*);

    typedef IsoDoseLevelVector ValueType;

    virtual ~IsoDoseLevelVectorProperty();

    const IsoDoseLevelVector * GetIsoDoseLevelVector() const;
    const IsoDoseLevelVector * GetValue() const;
    IsoDoseLevelVector * GetIsoDoseLevelVector();
    IsoDoseLevelVector * GetValue();


    void SetIsoDoseLevelVector(IsoDoseLevelVector* levelVector);
    void SetValue(IsoDoseLevelVector* levelVector);

    virtual std::string GetValueAsString() const;

    using BaseProperty::operator=;

private:

    // purposely not implemented
    IsoDoseLevelVectorProperty& operator=(const IsoDoseLevelVectorProperty&);

    itk::LightObject::Pointer InternalClone() const;

    virtual bool IsEqual(const BaseProperty& property) const;
    virtual bool Assign(const BaseProperty& property);

};

} // namespace mitk



#endif /* _MITK_DOSE_ISO_LEVEL_SET_PROPERTY_H_ */
