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

#include "mitkMAPRegistrationWrapper.h"

#include <mapExceptionObjectMacros.h>

mitk::MAPRegistrationWrapper::MAPRegistrationWrapper()
{
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

void mitk::MAPRegistrationWrapper::SetRegistration(map::core::RegistrationBase* pReg)
{
  m_spRegistration = pReg;
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
