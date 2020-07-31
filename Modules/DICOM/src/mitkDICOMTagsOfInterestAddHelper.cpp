/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMTagsOfInterestAddHelper.h"

#include <mitkIDICOMTagsOfInterest.h>

#include "usModuleContext.h"
#include "usGetModuleContext.h"

void mitk::DICOMTagsOfInterestAddHelper::Activate(us::ModuleContext* context, TagsOfInterestVector tags)
{
  if (!m_Active && nullptr != context)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Active = true;
    m_Context = context;
    m_TagsOfInterest = tags;

    // Listen for events pertaining to dictionary services.
    m_Context->AddServiceListener(this, &DICOMTagsOfInterestAddHelper::DICOMTagsOfInterestServiceChanged,
      std::string("(&(") + us::ServiceConstants::OBJECTCLASS() + "=" +
      us_service_interface_iid<IDICOMTagsOfInterest>() + "))");
    // Query for any service references matching any language.
    std::vector<us::ServiceReference<IDICOMTagsOfInterest> > refs =
      m_Context->GetServiceReferences<IDICOMTagsOfInterest>();
    if (!refs.empty())
    {
      for (const auto& ref : refs)
      {
        this->RegisterTagsOfInterest(context->GetService(ref));
        context->UngetService(ref);
      }
    }
  }
}

void mitk::DICOMTagsOfInterestAddHelper::Deactivate()
{
  if (m_Active)
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Active = false;
    if (nullptr != m_Context)
    {
      try
      {
          m_Context->RemoveServiceListener(this, &DICOMTagsOfInterestAddHelper::DICOMTagsOfInterestServiceChanged);
      }
      catch (...)
      {
        MITK_WARN << "Was not able to remove service listener from module context.";
      }
    }
  }
}

mitk::DICOMTagsOfInterestAddHelper::~DICOMTagsOfInterestAddHelper()
{
  if (m_Active)
  {
    MITK_ERROR << "DICOMTagsOfInterestAddHelper was not deactivated correctly befor its destructor was called.";
    auto context = us::GetModuleContext();
    //we cannot trust m_Context at this point anymore and have no means to validate it. So try to get the own module context
    //and to remove the listener via this context.
    if (nullptr != context)
    {
      try
      {
        context->RemoveServiceListener(this, &DICOMTagsOfInterestAddHelper::DICOMTagsOfInterestServiceChanged);
      }
      catch (...)
      {
        MITK_WARN << "Was not able to remove service listener from module context.";
      }
    }
  }
}

void mitk::DICOMTagsOfInterestAddHelper::RegisterTagsOfInterest(IDICOMTagsOfInterest* toiService) const
{
  if (nullptr != toiService)
  {
    for (const auto& tag : m_TagsOfInterest)
    {
      toiService->AddTagOfInterest(tag);
    }
  }
}

void mitk::DICOMTagsOfInterestAddHelper::DICOMTagsOfInterestServiceChanged(const us::ServiceEvent event)
{
  // If a DICOMTagsOfInterestService was registered, register all tags of interest.
  if (event.GetType() == us::ServiceEvent::REGISTERED)
  {
    if (nullptr != m_Context)
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      // Get a reference to the service object.
      us::ServiceReference<IDICOMTagsOfInterest> ref = event.GetServiceReference();
      this->RegisterTagsOfInterest(m_Context->GetService(ref));
      m_Context->UngetService(ref);
    }
    else
    {
      MITK_ERROR << "New DICOMTagsOfInterestService was registered, but no module context exists. Thus, no DICOM tags of interest where added.";
    }
  }
}
