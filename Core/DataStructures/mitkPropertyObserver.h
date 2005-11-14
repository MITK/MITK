#include <mitkBaseProperty.h>

namespace mitk {

/**
  \brief Convenience class to observe changes of a mitk::BaseProperty.

  This class registers itself as an ITK observer to a BaseProperty and gets
  informed of changes to the property. Whenever such a change occurrs, the virtual
  method PropertyChanged() or PropertyRemoved() is called. This way, derived 
  classes can implement behaviour for more specific properties (e.g. ColorProperty) 
  without the need to reimplement the Subject-Observer handling.

*/
class PropertyObserver
{
  public:

    PropertyObserver( mitk::BaseProperty* );
    virtual ~PropertyObserver();

    void OnModified(const itk::EventObject& e);
    virtual void PropertyChanged() = 0;

    void OnDelete(const itk::EventObject& e);
    virtual void PropertyRemoved() = 0;

  protected:

    mitk::BaseProperty* m_Property;
    unsigned long m_ModifiedObserverTag;
    unsigned long m_DeleteObserverTag;
};
  
}

