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


#include <mitkTestingMacros.h>

#include <mitkModule.h>
#include <mitkModuleContext.h>
#include <mitkGetModuleContext.h>

#include <mitkModulePropsInterface.h>

#include "mitkTestUtilSharedLibrary.cpp"


class ServiceListener
{

private:

  friend bool runLoadUnloadTest(const std::string&, int cnt, mitk::SharedLibraryHandle&,
                                const std::vector<mitk::ServiceEvent::Type>&);

  const bool checkUsingModules;
  std::vector<mitk::ServiceEvent> events;

  bool teststatus;

  mitk::ModuleContext* mc;

public:

  ServiceListener(mitk::ModuleContext* mc, bool checkUsingModules = true)
    : checkUsingModules(checkUsingModules), teststatus(true), mc(mc)
  {}

  bool getTestStatus() const
  {
    return teststatus;
  }

  void clearEvents()
  {
    events.clear();
  }

  bool checkEvents(const std::vector<mitk::ServiceEvent::Type>& eventTypes)
  {
    if (events.size() != eventTypes.size())
    {
      dumpEvents(eventTypes);
      return false;
    }

    for (std::size_t i=0; i < eventTypes.size(); ++i)
    {
      if (eventTypes[i] != events[i].GetType())
      {
        dumpEvents(eventTypes);
        return false;
      }
    }
    return true;
  }

  void serviceChanged(const mitk::ServiceEvent& evt)
  {
    events.push_back(evt);
    MITK_TEST_OUTPUT( << "ServiceEvent: " << evt );
    if (mitk::ServiceEvent::UNREGISTERING == evt.GetType())
    {
      mitk::ServiceReference sr = evt.GetServiceReference();

      // Validate that no module is marked as using the service
      std::vector<mitk::Module*> usingModules;
      sr.GetUsingModules(usingModules);
      if (checkUsingModules && !usingModules.empty())
      {
        teststatus = false;
        printUsingModules(sr, "*** Using modules (unreg) should be empty but is: ");
      }

      // Check if the service can be fetched
      itk::LightObject* service = mc->GetService(sr);
      sr.GetUsingModules(usingModules);
      // if (UNREGISTERSERVICE_VALID_DURING_UNREGISTERING) {
      // In this mode the service shall be obtainable during
      // unregistration.
      if (service == 0)
      {
        teststatus = false;
        MITK_TEST_OUTPUT( << "*** Service should be available to ServiceListener "
                          << "while handling unregistering event." );
      }
      MITK_TEST_OUTPUT( << "Service (unreg): " << service->GetNameOfClass() );
      if (checkUsingModules && usingModules.size() != 1)
      {
        teststatus = false;
        printUsingModules(sr, "*** One using module expected "
                          "(unreg, after getService), found: ");
      }
      else
      {
        printUsingModules(sr, "Using modules (unreg, after getService): ");
      }
      //    } else {
      //      // In this mode the service shall NOT be obtainable during
      //      // unregistration.
      //      if (null!=service) {
      //        teststatus = false;
      //        out.print("*** Service should not be available to ServiceListener "
      //                  +"while handling unregistering event.");
      //      }
      //      if (checkUsingBundles && null!=usingBundles) {
      //        teststatus = false;
      //        printUsingBundles(sr,
      //                          "*** Using bundles (unreg, after getService), "
      //                          +"should be null but is: ");
      //      } else {
      //        printUsingBundles(sr,
      //                          "Using bundles (unreg, after getService): null");
      //      }
      //    }
      mc->UngetService(sr);

      // Check that the UNREGISTERING service can not be looked up
      // using the service registry.
      try
      {
        long sid = mitk::any_cast<long>(sr.GetProperty(mitk::ServiceConstants::SERVICE_ID()));
        std::stringstream ss;
        ss << "(" << mitk::ServiceConstants::SERVICE_ID() << "=" << sid << ")";
        std::list<mitk::ServiceReference> srs = mc->GetServiceReferences("", ss.str());
        if (srs.empty())
        {
          MITK_TEST_OUTPUT( << "mitk::ServiceReference for UNREGISTERING service is not"
                            " found in the service registry; ok." );
        }
        else
        {
          teststatus = false;
          MITK_TEST_OUTPUT( << "*** mitk::ServiceReference for UNREGISTERING service,"
                            << sr << ", not found in the service registry; fail." );
          MITK_TEST_OUTPUT( << "Found the following Service references:") ;
          for(std::list<mitk::ServiceReference>::const_iterator sr = srs.begin();
              sr != srs.end(); ++sr)
          {
            MITK_TEST_OUTPUT( << (*sr) );
          }
        }
      }
      catch (const std::exception& e)
      {
        teststatus = false;
        MITK_TEST_OUTPUT( << "*** Unexpected excpetion when trying to lookup a"
                          " service while it is in state UNREGISTERING: "
                          << e.what() );
      }
    }
  }

  void printUsingModules(const mitk::ServiceReference& sr, const std::string& caption)
  {
    std::vector<mitk::Module*> usingModules;
    sr.GetUsingModules(usingModules);

    MITK_TEST_OUTPUT( << (caption.empty() ? "Using modules: " : caption) );
    for(std::vector<mitk::Module*>::const_iterator module = usingModules.begin();
        module != usingModules.end(); ++module)
    {
      MITK_TEST_OUTPUT( << "  -" << (*module) );
    }
  }

  // Print expected and actual service events.
  void dumpEvents(const std::vector<mitk::ServiceEvent::Type>& eventTypes)
  {
    std::size_t max = events.size() > eventTypes.size() ? events.size() : eventTypes.size();
    MITK_TEST_OUTPUT( << "Expected event type --  Actual event" );
    for (std::size_t i=0; i < max; ++i)
    {
      mitk::ServiceEvent evt = i < events.size() ? events[i] : mitk::ServiceEvent();
      if (i < eventTypes.size())
      {
        MITK_TEST_OUTPUT( << " " << eventTypes[i] << "--" << evt );
      }
      else
      {
        MITK_TEST_OUTPUT( << " " << "- NONE - " << "--" << evt );
      }
    }
  }

}; // end of class ServiceListener

bool runLoadUnloadTest(const std::string& name, int cnt, mitk::SharedLibraryHandle& target,
                       const std::vector<mitk::ServiceEvent::Type>& events)
{
  bool teststatus = true;

  mitk::ModuleContext* mc = mitk::GetModuleContext();

  for (int i = 0; i < cnt && teststatus; ++i)
  {
    ServiceListener sListen(mc);
    try
    {
      mc->AddServiceListener(&sListen, &ServiceListener::serviceChanged);
      //mc->AddServiceListener(mitk::MessageDelegate1<ServiceListener, const mitk::ServiceEvent&>(&sListen, &ServiceListener::serviceChanged));
    }
    catch (const std::logic_error& ise)
    {
      teststatus  = false;
      MITK_TEST_OUTPUT( << "service listener registration failed " << ise.what()
                        << " :" << name << ":FAIL" );
    }

    // Start the test target to get a service published.
    try
    {
      target.Load();
    }
    catch (const std::exception& e)
    {
      teststatus  = false;
      MITK_TEST_OUTPUT( << "Failed to load module, got exception: "
                        << e.what() << " + in " << name << ":FAIL" );
    }

    // Stop the test target to get a service unpublished.
    try
    {
      target.Unload();
    }
    catch (const std::exception& e)
    {
      teststatus  = false;
      MITK_TEST_OUTPUT( << "Failed to unload module, got exception: "
                        << e.what() << " + in " << name << ":FAIL" );
    }

    if (teststatus && !sListen.checkEvents(events))
    {
      teststatus  = false;
      MITK_TEST_OUTPUT( << "Service listener event notification error :"
                        << name << ":FAIL" );
    }

    try
    {
      mc->RemoveServiceListener(&sListen, &ServiceListener::serviceChanged);
      teststatus &= sListen.teststatus;
      sListen.clearEvents();
    }
    catch (const std::logic_error& ise)
    {
      teststatus  = false;
      MITK_TEST_OUTPUT( << "service listener removal failed " << ise.what()
                         << " :" << name << ":FAIL" );
    }
  }
  return teststatus;
}

void frameSL05a()
{
  std::vector<mitk::ServiceEvent::Type> events;
  events.push_back(mitk::ServiceEvent::REGISTERED);
  events.push_back(mitk::ServiceEvent::UNREGISTERING);
  mitk::SharedLibraryHandle libA("TestModuleA");
  bool testStatus = runLoadUnloadTest("FrameSL05a", 1, libA, events);
  MITK_TEST_CONDITION(testStatus, "FrameSL05a")
}

void frameSL10a()
{
  std::vector<mitk::ServiceEvent::Type> events;
  events.push_back(mitk::ServiceEvent::REGISTERED);
  events.push_back(mitk::ServiceEvent::UNREGISTERING);
  mitk::SharedLibraryHandle libA2("TestModuleA2");
  bool testStatus = runLoadUnloadTest("FrameSL10a", 1, libA2, events);
  MITK_TEST_CONDITION(testStatus, "FrameSL10a")
}

void frameSL25a()
{
  mitk::ModuleContext* mc = mitk::GetModuleContext();

  ServiceListener sListen(mc, false);
  try
  {
    mc->AddServiceListener(&sListen, &ServiceListener::serviceChanged);
  }
  catch (const std::logic_error& ise)
  {
    MITK_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  mitk::SharedLibraryHandle libSL1("TestModuleSL1");
  mitk::SharedLibraryHandle libSL3("TestModuleSL3");
  mitk::SharedLibraryHandle libSL4("TestModuleSL4");

  std::vector<mitk::ServiceEvent::Type> expectedServiceEventTypes;

  // Startup
  expectedServiceEventTypes.push_back(mitk::ServiceEvent::REGISTERED); // at start of libSL1
  expectedServiceEventTypes.push_back(mitk::ServiceEvent::REGISTERED); // mitk::FooService at start of libSL4
  expectedServiceEventTypes.push_back(mitk::ServiceEvent::REGISTERED); // at start of libSL3

  // Stop libSL4
  expectedServiceEventTypes.push_back(mitk::ServiceEvent::UNREGISTERING); // mitk::FooService at first stop of libSL4

  // Shutdown
  expectedServiceEventTypes.push_back(mitk::ServiceEvent::UNREGISTERING); // at stop of libSL1
  expectedServiceEventTypes.push_back(mitk::ServiceEvent::UNREGISTERING); // at stop of libSL3


  // Start libModuleTestSL1 to ensure that the Service interface is available.
  try
  {
    MITK_TEST_OUTPUT( << "Starting libModuleTestSL1: " << libSL1.GetAbsolutePath() );
    libSL1.Load();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_OUTPUT( << "Failed to load module, got exception: " << e.what() );
    throw;
  }

  // Start libModuleTestSL4 that will require the serivce interface and publish
  // ctkFooService
  try
  {
    MITK_TEST_OUTPUT( << "Starting libModuleTestSL4: " << libSL4.GetAbsolutePath() );
    libSL4.Load();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_OUTPUT( << "Failed to load module, got exception: " << e.what() );
    throw;
  }

  // Start libModuleTestSL3 that will require the serivce interface and get the service
  try
  {
    MITK_TEST_OUTPUT( << "Starting libModuleTestSL3: " << libSL3.GetAbsolutePath() );
    libSL3.Load();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_OUTPUT( << "Failed to load module, got exception: " << e.what() );
    throw;
  }

  // Check that libSL3 has been notified about the mitk::FooService.
  MITK_TEST_OUTPUT( << "Check that mitk::FooService is added to service tracker in libSL3" );
  try
  {
    mitk::ServiceReference libSL3SR = mc->GetServiceReference("mitk::ActivatorSL3");
    itk::LightObject* libSL3Activator = mc->GetService(libSL3SR);
    MITK_TEST_CONDITION_REQUIRED(libSL3Activator, "mitk::ActivatorSL3 service != 0");

    mitk::ModulePropsInterface* propsInterface = dynamic_cast<mitk::ModulePropsInterface*>(libSL3Activator);
    MITK_TEST_CONDITION_REQUIRED(propsInterface, "Cast to mitk::ModulePropsInterface");

    mitk::ModulePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    MITK_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceAdded");
    mitk::Any serviceAddedField3 = i->second;
    MITK_TEST_CONDITION_REQUIRED(!serviceAddedField3.Empty() && mitk::any_cast<bool>(serviceAddedField3),
                                 "libSL3 notified about presence of mitk::FooService");
    mc->UngetService(libSL3SR);
  }
  catch (const mitk::ServiceException& se)
  {
    MITK_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Check that libSL1 has been notified about the mitk::FooService.
  MITK_TEST_OUTPUT( << "Check that mitk::FooService is added to service tracker in libSL1" );
  try
  {
    mitk::ServiceReference libSL1SR = mc->GetServiceReference("mitk::ActivatorSL1");
    itk::LightObject* libSL1Activator = mc->GetService(libSL1SR);
    MITK_TEST_CONDITION_REQUIRED(libSL1Activator, "mitk::ActivatorSL1 service != 0");

    mitk::ModulePropsInterface* propsInterface = dynamic_cast<mitk::ModulePropsInterface*>(libSL1Activator);
    MITK_TEST_CONDITION_REQUIRED(propsInterface, "Cast to mitk::ModulePropsInterface");

    mitk::ModulePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceAdded");
    MITK_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceAdded");
    mitk::Any serviceAddedField1 = i->second;
    MITK_TEST_CONDITION_REQUIRED(!serviceAddedField1.Empty() && mitk::any_cast<bool>(serviceAddedField1),
                                 "libSL1 notified about presence of mitk::FooService");
    mc->UngetService(libSL1SR);
  }
  catch (const mitk::ServiceException& se)
  {
    MITK_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Stop the service provider: libSL4
  try
  {
    MITK_TEST_OUTPUT( << "Stop libSL4: " << libSL4.GetAbsolutePath() );
    libSL4.Unload();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_OUTPUT( << "Failed to unload module, got exception:" << e.what() );
    throw;
  }

  // Check that libSL3 has been notified about the removal of mitk::FooService.
  MITK_TEST_OUTPUT( << "Check that mitk::FooService is removed from service tracker in libSL3" );
  try
  {
    mitk::ServiceReference libSL3SR = mc->GetServiceReference("mitk::ActivatorSL3");
    itk::LightObject* libSL3Activator = mc->GetService(libSL3SR);
    MITK_TEST_CONDITION_REQUIRED(libSL3Activator, "mitk::ActivatorSL3 service != 0");

    mitk::ModulePropsInterface* propsInterface = dynamic_cast<mitk::ModulePropsInterface*>(libSL3Activator);
    MITK_TEST_CONDITION_REQUIRED(propsInterface, "Cast to mitk::ModulePropsInterface");

    mitk::ModulePropsInterface::Properties::const_iterator i = propsInterface->GetProperties().find("serviceRemoved");
    MITK_TEST_CONDITION_REQUIRED(i != propsInterface->GetProperties().end(), "Property serviceRemoved");

    mitk::Any serviceRemovedField3 = i->second;
    MITK_TEST_CONDITION(!serviceRemovedField3.Empty() && mitk::any_cast<bool>(serviceRemovedField3),
                        "libSL3 notified about removal of mitk::FooService");
    mc->UngetService(libSL3SR);
  }
  catch (const mitk::ServiceException& se)
  {
    MITK_TEST_FAILED_MSG( << "Failed to get service reference:" << se );
  }

  // Stop libSL1
  try
  {
    MITK_TEST_OUTPUT( << "Stop libSL1:" << libSL1.GetAbsolutePath() );
    libSL1.Unload();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_OUTPUT( << "Failed to unload module got exception" << e.what() );
    throw;
  }

  // Stop pSL3
  try
  {
    MITK_TEST_OUTPUT( << "Stop libSL3:" << libSL3.GetAbsolutePath() );
    libSL3.Unload();
  }
  catch (const std::exception& e)
  {
    MITK_TEST_OUTPUT( << "Failed to unload module got exception" << e.what() );
    throw;
  }

  // Check service events seen by this class
  MITK_TEST_OUTPUT( << "Checking ServiceEvents(ServiceListener):" );
  if (!sListen.checkEvents(expectedServiceEventTypes))
  {
    MITK_TEST_FAILED_MSG( << "Service listener event notification error" );
  }

  MITK_TEST_CONDITION_REQUIRED(sListen.getTestStatus(), "Service listener checks");
  try
  {
    mc->RemoveServiceListener(&sListen, &ServiceListener::serviceChanged);
    sListen.clearEvents();
  }
  catch (const std::logic_error& ise)
  {
    MITK_TEST_FAILED_MSG( << "service listener removal failed: " << ise.what() );
  }

}

int mitkServiceListenerTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ServiceListenerTest");

  frameSL05a();
  frameSL10a();
  frameSL25a();

  MITK_TEST_END()
}

