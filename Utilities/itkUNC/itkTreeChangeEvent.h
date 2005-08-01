/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __TreeChangeEvent_h
#define __TreeChangeEvent_h

#include "itkMacro.h"
#include <itkObject.h>
#include <itkTreeIteratorBase.h>

namespace itk
{
  template <class TTreeType>
  class TreeChangeEvent : public ModifiedEvent
  { 
  public: 
    typedef TreeChangeEvent Self; 
    typedef ModifiedEvent Superclass; 

    /** */
    TreeChangeEvent() 
      : m_ChangePosition( NULL ){}

    /** */
    TreeChangeEvent( const TreeIteratorBase<TTreeType>& position ) : 
      m_ChangePosition(&position) {} 

    /** */
    virtual ~TreeChangeEvent() {} 

    /** */
    virtual const char * GetEventName() const 
    { 
      return "TreeChangeEvent"; 
    } 

    /** */
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( *m_ChangePosition ); 
    } 

    /** */
    const TreeIteratorBase<TTreeType>& GetChangePosition() const 
    { 
      return *m_ChangePosition; 
    }

  private: 
    // TreeChangeEvent(const Self&); 
    void operator=(const Self&); 

  protected:
    const TreeIteratorBase<TTreeType>* m_ChangePosition;
  };

/**  */
  template <class TTreeType>
  class TreeAddEvent : public TreeChangeEvent<TTreeType>
  {  
  public:
    typedef TreeAddEvent Self; 
    typedef TreeChangeEvent<TTreeType> Superclass; 

    /** */
    TreeAddEvent() {}

    /** */
    TreeAddEvent( const TreeIteratorBase<TTreeType>& position ) : 
      TreeChangeEvent<TTreeType>(position) {} 

    /** */
    virtual const char * GetEventName() const 
    { 
      return "TreeAddEvent"; 
    } 

    /** */
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    /** */     
    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( *this->m_ChangePosition ); 
    } 
  
  };

/**  */
  template <class TTreeType>
  class TreeRemoveEvent : public TreeChangeEvent<TTreeType>
  {
  public:
    typedef TreeRemoveEvent Self; 
    typedef TreeChangeEvent<TTreeType> Superclass; 

    /** */
    TreeRemoveEvent(){}

    /** */
    TreeRemoveEvent( const TreeIteratorBase<TTreeType>& position ) : 
      TreeChangeEvent<TTreeType>(position) {} 

    /** */
    virtual const char * GetEventName() const 
    { 
      return "TreeRemoveEvent"; 
    } 

    /** */
    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    /** */     
    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( *this->m_ChangePosition ); 
    } 
  };


    // InvokeEvent(TreeChangeEvent<TTreeType>( position ));
  
} // namespace itk

#endif
