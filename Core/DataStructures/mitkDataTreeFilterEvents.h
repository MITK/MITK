#ifndef MITK_DATATREEFILTEREVENTS_H_INCLUDED
#define MITK_DATATREEFILTEREVENTS_H_INCLUDED

#include <itkEventObject.h>

namespace mitk {

class DataTreeFilter::Item;
  
//------ TreeFilterUpdateAllEvent --------------------------------------------------------

itkEventMacro( TreeFilterUpdateAllEvent, itk::ModifiedEvent );
itkEventMacro( TreeFilterRemoveAllEvent, itk::ModifiedEvent );

//------ TreeFilterItemEvent -------------------------------------------------------------
  class TreeFilterItemEvent : public itk::ModifiedEvent
  { 
  public: 
    typedef TreeFilterItemEvent Self; 
    typedef ModifiedEvent Superclass; 

    TreeFilterItemEvent() 
      : m_ChangedItem( NULL ){}

    TreeFilterItemEvent( const mitk::DataTreeFilter::Item* item ) : 
      m_ChangedItem(item) {} 

    virtual ~TreeFilterItemEvent() {} 

    virtual const char * GetEventName() const 
    { 
      return "TreeFilterItemEvent"; 
    } 

    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem ); 
    } 

    const mitk::DataTreeFilter::Item* GetChangedItem() const 
    { 
      return m_ChangedItem; 
    }

  protected:
    const mitk::DataTreeFilter::Item* m_ChangedItem;
  
  private: 
    TreeFilterItemEvent(const Self&); 
    void operator=(const Self&); 

  };


//------ TreeFilterItemAddedEvent --------------------------------------------------------
  class TreeFilterItemAddedEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterItemAddedEvent Self; 
    typedef ModifiedEvent Superclass; 

    TreeFilterItemAddedEvent() : TreeFilterItemEvent() {}
    TreeFilterItemAddedEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterItemAddedEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterItemAddedEvent"; 
    } 
  
  private: 
    TreeFilterItemAddedEvent(const Self&); 
    void operator=(const Self&); 
    
  };

//------ TreeFilterSelectionChanged ------------------------------------------------------
  class TreeFilterSelectionChangedEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterSelectionChangedEvent Self; 
    typedef ModifiedEvent Superclass; 

    TreeFilterSelectionChangedEvent() : TreeFilterItemEvent() {}
    TreeFilterSelectionChangedEvent(const mitk::DataTreeFilter::Item* item, bool selected) : TreeFilterItemEvent(item), m_Selected(selected) {}
    
    virtual ~TreeFilterSelectionChangedEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterSelectionChangedEvent"; 
    } 

    virtual bool IsSelected() { return m_Selected; }
  
  private: 
    TreeFilterSelectionChangedEvent(const Self&); 
    void operator=(const Self&); 

    bool m_Selected;
    
  };

//------ TreeFilterItemChangedEvent ------------------------------------------------------
  class TreeFilterItemChangedEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterItemChangedEvent Self; 
    typedef ModifiedEvent Superclass; 

    TreeFilterItemChangedEvent() : TreeFilterItemEvent() {}
    TreeFilterItemChangedEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterItemChangedEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterItemChangedEvent"; 
    } 
  
  private: 
    TreeFilterItemChangedEvent(const Self&); 
    void operator=(const Self&); 
    
  };
  
  
//------ TreeFilterRemoveItemEvent -------------------------------------------------------
  class TreeFilterRemoveItemEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterRemoveItemEvent Self; 
    typedef ModifiedEvent Superclass; 

    TreeFilterRemoveItemEvent() : TreeFilterItemEvent() {}
    TreeFilterRemoveItemEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterRemoveItemEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterRemoveItemEvent"; 
    } 
  
  private: 
    TreeFilterRemoveItemEvent(const Self&); 
    void operator=(const Self&); 
    
  };

//------ TreeFilterRemoveChildrenEvent -------------------------------------------------------
  class TreeFilterRemoveChildrenEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterRemoveChildrenEvent Self; 
    typedef ModifiedEvent Superclass; 

    TreeFilterRemoveChildrenEvent() : TreeFilterItemEvent() {}
    TreeFilterRemoveChildrenEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterRemoveChildrenEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterRemoveChildrenEvent"; 
    } 
  
  private: 
    TreeFilterRemoveChildrenEvent(const Self&); 
    void operator=(const Self&); 
    
  };

}

#endif

