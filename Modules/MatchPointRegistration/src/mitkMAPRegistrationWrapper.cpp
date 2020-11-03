/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMAPRegistrationWrapper.h"

#include <mapExceptionObjectMacros.h>
#include <mapRegistrationManipulator.h>

mitk::MAPRegistrationWrapper::MAPRegistrationWrapper(map::core::RegistrationBase* registration) : m_spRegistration(registration)
{
  if (registration == nullptr)
  {
    mitkThrow() << "Error. Cannot create MAPRegistrationWrapper with invalid registration instance (nullptr).";
  }

  Identifiable::SetUID(registration->getRegistrationUID());
}

mitk::MAPRegistrationWrapper::~MAPRegistrationWrapper()
{
}

void mitk::MAPRegistrationWrapper::SetRequestedRegionToLargestPossibleRegion()
{
    //nothing to do
}

bool mitk::MAPRegistrationWrapper::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

bool mitk::MAPRegistrationWrapper::VerifyRequestedRegion()
{
    return true;
}

void mitk::MAPRegistrationWrapper::SetRequestedRegion(const itk::DataObject*)
{
    //nothing to do
}

unsigned int mitk::MAPRegistrationWrapper::GetMovingDimensions() const
{
    if (m_spRegistration.IsNull())
    {
        mitkThrow()<< "Error. Cannot return moving dimension. Wrapper points to invalid registration (nullptr).";
    }
    return m_spRegistration->getMovingDimensions();
}


unsigned int mitk::MAPRegistrationWrapper::GetTargetDimensions() const
{
    if (m_spRegistration.IsNull())
    {
        mitkThrow()<< "Error. Cannot return target dimension. Wrapper points to invalid registration (nullptr).";
    }
    return m_spRegistration->getTargetDimensions();
}

const mitk::MAPRegistrationWrapper::TagMapType& mitk::MAPRegistrationWrapper::GetTags() const
{
    if (m_spRegistration.IsNull())
    {
        mitkThrow()<< "Error. Cannot return registration tags. Wrapper points to invalid registration (nullptr).";
    }
    return m_spRegistration->getTags();
}

bool mitk::MAPRegistrationWrapper::GetTagValue(const TagType & tag, ValueType & value) const
{
    if (m_spRegistration.IsNull())
    {
        mitkThrow()<< "Error. Cannot return registration tag value. Wrapper points to invalid registration (nullptr). Tag: " << tag;
    }
    return m_spRegistration->getTagValue(tag,value);
}

bool mitk::MAPRegistrationWrapper::HasLimitedTargetRepresentation() const
{
    if (m_spRegistration.IsNull())
    {
        mitkThrow()<< "Error. Cannot determin HasLimitedTargetRepresentation(). Wrapper points to invalid registration (nullptr).";
    }
    return m_spRegistration->hasLimitedTargetRepresentation();
}

bool mitk::MAPRegistrationWrapper::HasLimitedMovingRepresentation() const
{
    if (m_spRegistration.IsNull())
    {
        mitkThrow()<< "Error. Cannot determin HasLimitedMovingRepresentation(). Wrapper points to invalid registration (nullptr).";
    }
    return m_spRegistration->hasLimitedMovingRepresentation();
}

map::core::RegistrationBase* mitk::MAPRegistrationWrapper::GetRegistration()
{
    return m_spRegistration;
}

const map::core::RegistrationBase* mitk::MAPRegistrationWrapper::GetRegistration() const
{
    return m_spRegistration;
}

void mitk::MAPRegistrationWrapper::PrintSelf (std::ostream &os, itk::Indent indent) const
{
    Superclass::PrintSelf(os,indent);
    if (m_spRegistration.IsNull())
    {
        os<< "Error. Wrapper points to invalid registration (nullptr).";
    }
    else
    {
        os<<std::endl<<indent<<"MatchPoint registration instance:";
        m_spRegistration->Print(os,indent.GetNextIndent());

        typedef map::core::Registration<3,3> CastedRegType;
        const CastedRegType* pCastedReg = dynamic_cast<const CastedRegType*>(m_spRegistration.GetPointer());

        os<<std::endl<<indent<<"MatchPoint registration direct kernel instance:";
        pCastedReg->getDirectMapping().Print(os,indent.GetNextIndent());

        os<<std::endl<<indent<<"MatchPoint registration inverse kernel instance:";
        pCastedReg->getInverseMapping().Print(os,indent.GetNextIndent());

    }
}

void  mitk::MAPRegistrationWrapper::SetUID(const UIDType& uid)
{
  if (m_spRegistration.IsNull())
  {
    mitkThrow() << "Error. Cannot set UID. Wrapper points to invalid registration (nullptr).";
  }
  Identifiable::SetUID(uid);
  ::map::core::RegistrationBaseManipulator manip(m_spRegistration);
  manip.getTagValues()[::map::tags::RegistrationUID] = uid;
};

mitk::Identifiable::UIDType mitk::MAPRegistrationWrapper::GetUID() const
{
  if (m_spRegistration.IsNull())
  {
    mitkThrow() << "Error. Cannot return UID. Wrapper points to invalid registration (nullptr).";
  }
  return m_spRegistration->getRegistrationUID();
};
