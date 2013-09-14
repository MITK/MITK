#include <usServiceTracker.h>
#include <usGetModuleContext.h>

US_USE_NAMESPACE

struct IFooService {};

US_DECLARE_SERVICE_INTERFACE(IFooService, "org.cppmicroservices.snippets.IFooService")

///! [tt]
struct MyTrackedClass { /* ... */ };
//! [tt]

//! [ttt]
struct MyTrackedClassTraits : public TrackedTypeTraitsBase<MyTrackedClass, MyTrackedClassTraits>
{
  static bool IsValid(const TrackedType&)
  {
    // Dummy implementation
    return true;
  }

  static void Dispose(TrackedType&)
  {}

  static TrackedType DefaultValue()
  {
    return TrackedType();
  }
};
//! [ttt]

//! [customizer]
struct MyTrackingCustomizer : public ServiceTrackerCustomizer<IFooService, MyTrackedClass>
{
  virtual MyTrackedClass AddingService(const ServiceReferenceT&)
  {
    return MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceT&, MyTrackedClass)
  {
  }

  virtual void RemovedService(const ServiceReferenceT&, MyTrackedClass)
  {
  }
};
//! [customizer]

struct MyTrackingPointerCustomizer : public ServiceTrackerCustomizer<IFooService, MyTrackedClass*>
{
  virtual MyTrackedClass* AddingService(const ServiceReferenceT&)
  {
    return new MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceT&, MyTrackedClass*)
  {
  }

  virtual void RemovedService(const ServiceReferenceT&, MyTrackedClass*)
  {
  }
};

// For compilation test purposes only
struct MyTrackingCustomizerVoid : public ServiceTrackerCustomizer<void, MyTrackedClass>
{
  virtual MyTrackedClass AddingService(const ServiceReferenceT&)
  {
    return MyTrackedClass();
  }

  virtual void ModifiedService(const ServiceReferenceT&, MyTrackedClass)
  {
  }

  virtual void RemovedService(const ServiceReferenceT&, MyTrackedClass)
  {
  }
};

int main(int /*argc*/, char* /*argv*/[])
{
  {
//! [tracker]
MyTrackingCustomizer myCustomizer;
ServiceTracker<IFooService, MyTrackedClassTraits> tracker(GetModuleContext(), &myCustomizer);
//! [tracker]
  }

  {
//! [tracker2]
MyTrackingPointerCustomizer myCustomizer;
ServiceTracker<IFooService, TrackedTypeTraits<IFooService,MyTrackedClass*> > tracker(GetModuleContext(), &myCustomizer);
//! [tracker2]
  }

  // For compilation test purposes only
  MyTrackingCustomizerVoid myCustomizer2;
  ServiceTracker<void, MyTrackedClassTraits> tracker2(GetModuleContext(), &myCustomizer2);
  ServiceTracker<void, TrackedTypeTraits<void,MyTrackedClass*> > tracker3(GetModuleContext());

  return 0;
}

#include <usModuleInitialization.h>

US_INITIALIZE_EXECUTABLE("uServices-modulecontext")
