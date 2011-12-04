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

#include <mitkModule.h>
#include <mitkModuleContext.h>
#include <mitkGetModuleContext.h>
#include <mitkModuleRegistry.h>
#include "mitkTestUtilSharedLibrary.cpp"

#include <mitkTestingMacros.h>
#include <mitkLogMacros.h>

#include "TestModules/libA/mitkTestModuleAService.h"

class ModuleListener {

public:

  ModuleListener(mitk::ModuleContext* mc) : mc(mc)
  {}

  void ModuleChanged(const mitk::ModuleEvent& event)
  {
    moduleEvents.push_back(event);
    MITK_DEBUG << "ModuleEvent:" << event;
  }

  void ServiceChanged(const mitk::ServiceEvent& event)
  {
    serviceEvents.push_back(event);
    MITK_DEBUG << "ServiceEvent:" << event;
  }

  mitk::ModuleEvent GetModuleEvent() const
  {
    if (moduleEvents.empty())
    {
      return mitk::ModuleEvent();
    }
    return moduleEvents.back();
  }

  mitk::ServiceEvent GetServiceEvent() const
  {
    if (serviceEvents.empty())
    {
      return mitk::ServiceEvent();
    }
    return serviceEvents.back();
  }

  bool CheckListenerEvents(
      bool pexp, mitk::ModuleEvent::Type ptype,
      bool sexp, mitk::ServiceEvent::Type stype,
      mitk::Module* moduleX, mitk::ServiceReference* servX)
  {
    std::vector<mitk::ModuleEvent> pEvts;
    std::vector<mitk::ServiceEvent> seEvts;

    if (pexp) pEvts.push_back(mitk::ModuleEvent(ptype, moduleX));
    if (sexp) seEvts.push_back(mitk::ServiceEvent(stype, *servX));

    return CheckListenerEvents(pEvts, seEvts);
  }

  bool CheckListenerEvents(
      const std::vector<mitk::ModuleEvent>& pEvts,
      const std::vector<mitk::ServiceEvent>& seEvts)
  {
    bool listenState = true; // assume everything will work

    if (pEvts.size() != moduleEvents.size())
    {
      listenState = false;
      MITK_DEBUG << "*** Module event mismatch: expected "
          << pEvts.size() << " event(s), found "
          << moduleEvents.size() << " event(s).";

      const std::size_t max = pEvts.size() > moduleEvents.size() ? pEvts.size() : moduleEvents.size();
      for (std::size_t i = 0; i < max; ++i)
      {
        const mitk::ModuleEvent& pE = i < pEvts.size() ? pEvts[i] : mitk::ModuleEvent();
        const mitk::ModuleEvent& pR = i < moduleEvents.size() ? moduleEvents[i] : mitk::ModuleEvent();
        MITK_DEBUG << "    " << pE << " - " << pR;
      }
    }
    else
    {
      for (std::size_t i = 0; i < pEvts.size(); ++i)
      {
        const mitk::ModuleEvent& pE = pEvts[i];
        const mitk::ModuleEvent& pR = moduleEvents[i];
        if (pE.GetType() != pR.GetType()
          || pE.GetModule() != pR.GetModule())
        {
          listenState = false;
          MITK_DEBUG << "*** Wrong module event: " << pR << " expected " << pE;
        }
      }
    }

    if (seEvts.size() != serviceEvents.size())
    {
      listenState = false;
      MITK_DEBUG << "*** Service event mismatch: expected "
          << seEvts.size() << " event(s), found "
          << serviceEvents.size() << " event(s).";

      const std::size_t max = seEvts.size() > serviceEvents.size()
                      ? seEvts.size() : serviceEvents.size();
      for (std::size_t i = 0; i < max; ++i)
      {
        const mitk::ServiceEvent& seE = i < seEvts.size() ? seEvts[i] : mitk::ServiceEvent();
        const mitk::ServiceEvent& seR = i < serviceEvents.size() ? serviceEvents[i] : mitk::ServiceEvent();
        MITK_DEBUG << "    " << seE << " - " << seR;
      }
    }
    else
    {
      for (std::size_t i = 0; i < seEvts.size(); ++i)
      {
        const mitk::ServiceEvent& seE = seEvts[i];
        const mitk::ServiceEvent& seR = serviceEvents[i];
        if (seE.GetType() != seR.GetType()
          || (!(seE.GetServiceReference() == seR.GetServiceReference())))
        {
          listenState = false;
          MITK_DEBUG << "*** Wrong service event: " << seR << " expected " << seE;
        }
      }
    }

    moduleEvents.clear();
    serviceEvents.clear();
    return listenState;
  }

private:

  mitk::ModuleContext* const mc;

  std::vector<mitk::ServiceEvent> serviceEvents;
  std::vector<mitk::ModuleEvent> moduleEvents;
};

// Verify information from the ModuleInfo struct
void frame005a(mitk::ModuleContext* mc)
{
  mitk::Module* m = mc->GetModule();

  // check expected headers

  MITK_TEST_CONDITION("MitkTestDriver" == m->GetName(), "Test module name");
//  MITK_DEBUG << "Version: " << m->GetVersion();
  MITK_TEST_CONDITION(mitk::ModuleVersion(0,1,0) == m->GetVersion(), "Test module version")
}

// Get context id, location and status of the module
void frame010a(mitk::ModuleContext* mc)
{
  mitk::Module* m = mc->GetModule();

  long int contextid = m->GetModuleId();
  MITK_DEBUG << "CONTEXT ID:" << contextid;

  std::string location = m->GetLocation();
  MITK_DEBUG << "LOCATION:" << location;
  MITK_TEST_CONDITION(!location.empty(), "Test for non-empty module location")

  MITK_TEST_CONDITION(m->IsLoaded(), "Test for loaded flag")
}

//----------------------------------------------------------------------------
//Test result of GetService(mitk::ServiceReference()). Should throw std::invalid_argument
void frame018a(mitk::ModuleContext* mc)
{
  try
  {
    itk::LightObject* obj = mc->GetService(mitk::ServiceReference());
    MITK_DEBUG << "Got service object = " << obj->GetNameOfClass() << ", excpected std::invalid_argument exception";
    MITK_TEST_FAILED_MSG(<< "Got service object, excpected std::invalid_argument exception")
  }
  catch (const std::invalid_argument& )
  {}
  catch (...)
  {
    MITK_TEST_FAILED_MSG(<< "Got wrong exception, expected std::invalid_argument")
  }
}

// Load libA and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(mitk::ModuleContext* mc, ModuleListener& listener, mitk::SharedLibraryHandle& libA)
{
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  mitk::Module* moduleA = mitk::ModuleRegistry::GetModule("TestModuleA");
  MITK_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing moudle TestModuleA")

  MITK_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  // Check if libA registered the expected service
  try
  {
    mitk::ServiceReference sr1 = mc->GetServiceReference("org.mitk.TestModuleAService");
    itk::LightObject* o1 = mc->GetService(sr1);
    MITK_TEST_CONDITION(o1 != 0, "Test if service object found");

    try
    {
      MITK_TEST_CONDITION(mc->UngetService(sr1), "Test if Service UnGet returns true");
    }
    catch (const std::logic_error le)
    {
      MITK_TEST_FAILED_MSG(<< "UnGetService exception: " << le.what())
    }

    // check the listeners for events
    std::vector<mitk::ModuleEvent> pEvts;
    pEvts.push_back(mitk::ModuleEvent(mitk::ModuleEvent::LOADING, moduleA));
    pEvts.push_back(mitk::ModuleEvent(mitk::ModuleEvent::LOADED, moduleA));

    std::vector<mitk::ServiceEvent> seEvts;
    seEvts.push_back(mitk::ServiceEvent(mitk::ServiceEvent::REGISTERED, sr1));

    MITK_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const mitk::ServiceException& /*se*/)
  {
    MITK_TEST_FAILED_MSG(<< "test moudle, expected service not found");
  }

  MITK_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");
}


// Unload libA and check for correct events
void frame030b(mitk::ModuleContext* mc, ModuleListener& listener, mitk::SharedLibraryHandle& libA)
{
  mitk::Module* moduleA = mitk::ModuleRegistry::GetModule("TestModuleA");
  MITK_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for non-null module")

  mitk::ServiceReference sr1
      = mc->GetServiceReference("org.mitk.TestModuleAService");
  MITK_TEST_CONDITION(sr1, "Test for valid service reference")

  mitk::TestModuleAService* moduleAService = mc->GetService<mitk::TestModuleAService>(sr1);
  MITK_TEST_CONDITION(moduleAService != 0, "Test for valid service object")

  bool unloadCalled = false;
  moduleAService->SetUnloadedFlag(&unloadCalled);

  try
  {
    libA.Unload();
    MITK_TEST_CONDITION(unloadCalled == true, "Test if the mitk::ModuleActivator::Unload() method was called")
    MITK_TEST_CONDITION(moduleA->IsLoaded() == false, "Test for unloaded state")
  }
  catch (const std::exception& e)
  {
    MITK_TEST_FAILED_MSG(<< "UnLoad module exception: " << e.what())
  }

  std::vector<mitk::ModuleEvent> pEvts;
  pEvts.push_back(mitk::ModuleEvent(mitk::ModuleEvent::UNLOADING, moduleA));
  pEvts.push_back(mitk::ModuleEvent(mitk::ModuleEvent::UNLOADED, moduleA));

  std::vector<mitk::ServiceEvent> seEvts;
  seEvts.push_back(mitk::ServiceEvent(mitk::ServiceEvent::UNREGISTERING, sr1));

  MITK_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}


struct LocalListener {
  void ServiceChanged(const mitk::ServiceEvent&) {}
};

// Add a service listener with a broken LDAP filter to Get an exception
void frame045a(mitk::ModuleContext* mc)
{
  LocalListener sListen1;
  std::string brokenFilter = "A broken LDAP filter";

  try
  {
    mc->AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter);
  }
  catch (const std::invalid_argument& /*ia*/)
  {
    //assertEquals("InvalidSyntaxException.GetFilter should be same as input string", brokenFilter, ise.GetFilter());
  }
  catch (...)
  {
    MITK_TEST_FAILED_MSG(<< "test module, wrong exception on broken LDAP filter:");
  }
}


int mitkModuleTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ModuleTest");

  mitk::ModuleContext* mc = mitk::GetModuleContext();
  ModuleListener listener(mc);

  try
  {
    mc->AddModuleListener(&listener, &ModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    MITK_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  try
  {
    mc->AddServiceListener(&listener, &ModuleListener::ServiceChanged);
  }
  catch (const std::logic_error& ise)
  {
    MITK_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  frame005a(mc);
  frame010a(mc);
  frame018a(mc);

  mitk::SharedLibraryHandle libA("TestModuleA");
  frame020a(mc, listener, libA);
  frame030b(mc, listener, libA);

  frame045a(mc);

  mc->RemoveModuleListener(&listener, &ModuleListener::ModuleChanged);
  mc->RemoveServiceListener(&listener, &ModuleListener::ServiceChanged);

  MITK_TEST_END()
}
