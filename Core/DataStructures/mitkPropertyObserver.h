#ifndef MITK_BASEPROPERTYOBSERVER_H_INCLUDED
#define MITK_BASEPROPERTYOBSERVER_H_INCLUDED

#include <itkEventObject.h>

namespace mitk {

/**
  \brief Convenience class to observe changes of a mitk::BaseProperty.

  This class registers itself as an ITK observer to a BaseProperty and gets
  informed of changes to the property. Whenever such a change occurrs, the virtual
  method PropertyChanged() or PropertyRemoved() is called. This way, derived 
  classes can implement behaviour for more specific properties (e.g. ColorProperty) 
  without the need to reimplement the Subject-Observer handling.

*/
class PropertyObserver {};

class BaseProperty;
  
class PropertyView : public PropertyObserver
{
  public:

    PropertyView( const mitk::BaseProperty* );
    virtual ~PropertyView();

    void OnModified(const itk::EventObject& e);

    void OnDelete(const itk::EventObject& e);

  protected:

    virtual void PropertyChanged() = 0;
    virtual void PropertyRemoved() = 0;

    void BeginModifyProperty();
    void EndModifyProperty();
    
    const mitk::BaseProperty* m_Property;
    unsigned long m_ModifiedTag;
    unsigned long m_DeleteTag;
};

class PropertyEditor : public PropertyObserver
{
  public:

    PropertyEditor( mitk::BaseProperty* );
    virtual ~PropertyEditor();

    void OnModified(const itk::EventObject& e);
    virtual void PropertyChanged() = 0;

    void OnDelete(const itk::EventObject& e);
    virtual void PropertyRemoved() = 0;

  protected:
    
    void BeginModifyProperty();
    void EndModifyProperty();

    mitk::BaseProperty* m_Property;
    unsigned long m_ModifiedTag;
    unsigned long m_DeleteTag;
};
  
}

#endif

