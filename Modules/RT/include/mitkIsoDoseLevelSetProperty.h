/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIsoDoseLevelSetProperty_h
#define mitkIsoDoseLevelSetProperty_h

#include "mitkBaseProperty.h"
#include "mitkIsoDoseLevelCollections.h"
#include "MitkRTExports.h"

namespace mitk {

/**
\brief Property class for dose iso level sets.
*/
class MITKRT_EXPORT IsoDoseLevelSetProperty : public BaseProperty
{

protected:
    IsoDoseLevelSet::Pointer m_IsoLevelSet;

    IsoDoseLevelSetProperty();

    explicit IsoDoseLevelSetProperty(const IsoDoseLevelSetProperty& other);

    explicit IsoDoseLevelSetProperty(IsoDoseLevelSet* levelSet);

public:
    mitkClassMacro(IsoDoseLevelSetProperty, BaseProperty);

    itkNewMacro(IsoDoseLevelSetProperty);
    mitkNewMacro1Param(IsoDoseLevelSetProperty, IsoDoseLevelSet*);

    typedef IsoDoseLevelSet ValueType;

    ~IsoDoseLevelSetProperty() override;

    const IsoDoseLevelSet * GetIsoDoseLevelSet() const;
    const IsoDoseLevelSet * GetValue() const;
    IsoDoseLevelSet * GetIsoDoseLevelSet();
    IsoDoseLevelSet * GetValue();


    void SetIsoDoseLevelSet(IsoDoseLevelSet* levelSet);
    void SetValue(IsoDoseLevelSet* levelSet);

    std::string GetValueAsString() const override;

    using BaseProperty::operator=;

private:

    itk::LightObject::Pointer InternalClone() const override;

    bool IsEqual(const BaseProperty& property) const override;
    bool Assign(const BaseProperty& property) override;

};

} // namespace mitk


#endif
