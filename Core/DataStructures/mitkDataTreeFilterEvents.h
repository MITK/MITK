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
    typedef itk::ModifiedEvent Superclass; 

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
    
    TreeFilterItemEvent(const Self& s) : itk::ModifiedEvent(s), m_ChangedItem(s.m_ChangedItem) {}; 

  protected:
    const mitk::DataTreeFilter::Item* m_ChangedItem;
  
  private: 
    void operator=(const Self&); 

  };

//------ TreeFilterEvent --------------------------------------------------------
  class TreeFilterNewItemEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterNewItemEvent Self; 
    typedef TreeFilterItemEvent Superclass; 

    TreeFilterNewItemEvent() : TreeFilterItemEvent() {}
    TreeFilterNewItemEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterNewItemEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterNewItemEvent"; 
    } 
    
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem ); 
    } 
    
    TreeFilterNewItemEvent(const Self& s) : TreeFilterItemEvent(s) {}; 
  
  private: 
    void operator=(const Self&); 
    
  };


//------ TreeFilterItemAddedEvent --------------------------------------------------------
  class TreeFilterItemAddedEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterItemAddedEvent Self; 
    typedef TreeFilterItemEvent Superclass; 

    TreeFilterItemAddedEvent() : TreeFilterItemEvent() {}
    TreeFilterItemAddedEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterItemAddedEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterItemAddedEvent"; 
    } 
    
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem ); 
    } 
    
    TreeFilterItemAddedEvent(const Self& s) : TreeFilterItemEvent(s) {}; 
  
  private: 
    void operator=(const Self&); 
    
  };

//------ TreeFilterSelectionChangedEvent -------------------------------------------------
  class TreeFilterSelectionChangedEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterSelectionChangedEvent Self; 
    typedef TreeFilterItemEvent Superclass; 

    TreeFilterSelectionChangedEvent() : TreeFilterItemEvent() {}
    TreeFilterSelectionChangedEvent(const mitk::DataTreeFilter::Item* item, bool selected) : TreeFilterItemEvent(item), m_Selected(selected) {}
    
    virtual ~TreeFilterSelectionChangedEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterSelectionChangedEvent"; 
    } 
    
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem, m_Selected ); 
    } 

    virtual bool IsSelected() const { return m_Selected; }
  
    TreeFilterSelectionChangedEvent(const Self&s) : TreeFilterItemEvent(s) {}; 
  private: 
    void operator=(const Self&); 

    bool m_Selected;
    
  };

  //------ TreeFilterItemChangedEvent ------------------------------------------------------
  class TreeFilterItemChangedEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterItemChangedEvent Self; 
    typedef TreeFilterItemEvent Superclass; 

    TreeFilterItemChangedEvent() : TreeFilterItemEvent() {}
    TreeFilterItemChangedEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterItemChangedEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterItemChangedEvent"; 
    } 
    
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem ); 
    } 
    
    TreeFilterItemChangedEvent(const Self&s) : TreeFilterItemEvent(s) {}; 
  
  private: 
    void operator=(const Self&); 
    
  };
  
//------ TreeFilterRemoveItemEvent -------------------------------------------------------
  class TreeFilterRemoveItemEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterRemoveItemEvent Self; 
    typedef TreeFilterItemEvent Superclass; 

    TreeFilterRemoveItemEvent() : TreeFilterItemEvent() {}
    TreeFilterRemoveItemEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterRemoveItemEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterRemoveItemEvent"; 
    } 
    
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem ); 
    } 
    
    TreeFilterRemoveItemEvent(const Self&s) : TreeFilterItemEvent(s) {}; 
  
  private: 
    void operator=(const Self&); 
    
  };

//------ TreeFilterRemoveChildrenEvent -------------------------------------------------------
  class TreeFilterRemoveChildrenEvent : public TreeFilterItemEvent 
  {
  public:
    typedef TreeFilterRemoveChildrenEvent Self; 
    typedef TreeFilterItemEvent Superclass; 

    TreeFilterRemoveChildrenEvent() : TreeFilterItemEvent() {}
    TreeFilterRemoveChildrenEvent(const mitk::DataTreeFilter::Item* item) : TreeFilterItemEvent(item) {}
    
    virtual ~TreeFilterRemoveChildrenEvent() {} 
    
    virtual const char * GetEventName() const 
    { 
      return "TreeFilterRemoveChildrenEvent"; 
    } 
    
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_ChangedItem ); 
    } 
  
    TreeFilterRemoveChildrenEvent(const Self&s) : TreeFilterItemEvent(s) {}; 
    
  private: 
    void operator=(const Self&); 
    
  };
}

#endif

