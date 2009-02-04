/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: XXXXXX $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include <exception>
#include <iostream>
#include "cherryObject.h"
#include "cherryOSGiDll.h"
#include "Poco/Exception.h"

namespace cherry {

/** \class Weakpointer
 *  \brief implements a WeakPointer class to deal with circular reference problems.
 *
 * 
 * The Weakpointer Object can be described as a testing module before returning a smartpointer.
 * The Weakpointer checks if the Object it is referring to is still alive. If not the Weakpointer returns an Exception if the log method is used. 
 * If the Object is still alive the Weakpointer class creates a smartpointer of the referring object. 
 *
 */

template<typename TObjectType>
class WeakPointer{

public:

  typedef TObjectType ObjectType;
  friend class MessageDelegate<WeakPointer>;

  /** Constructor */
  explicit WeakPointer(cherry::SmartPointer<ObjectType> sptr):m_Pointer(sptr.GetPointer()) {

  m_Pointer->AddDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister)); 
  
   }
    /** Default Constructor */
  WeakPointer():m_Pointer(0) {
  
  
  }

  /** Copy constructor  */
  WeakPointer(const WeakPointer<ObjectType> &p) :
    m_Pointer(p.m_Pointer)
  {
    m_Pointer->AddDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister));  
  }
  
  
  WeakPointer &operator =(const SmartPointer<ObjectType> &r)
  {
   if(m_Pointer)
   m_Pointer->RemoveDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister));
   m_Pointer = r.GetPointer(); 
   if(m_Pointer)
   m_Pointer->AddDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister));
   
   return *this;
  }
    
    /** look method is used to access the referring object  */
   SmartPointer<ObjectType> lock(){

    if (m_Pointer == 0 ) {
      throw Poco::NullPointerException("Weakpointer trying to point to a non exsisting object");
    }
    else {
      return SmartPointer<ObjectType> (m_Pointer) ; 
    }

  }
 
   
                                    
  WeakPointer& operator=(const WeakPointer& other) 
  {
    if(m_Pointer)
      m_Pointer->RemoveDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister));

    this->m_Pointer = other.m_Pointer;
    if(m_Pointer)
      m_Pointer->AddDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister));

    return *this;
  }


  /** Destructor */
  ~ WeakPointer () {
    
   if(m_Pointer)
      m_Pointer->RemoveDestroyListener(MessageDelegate<WeakPointer>(this, &WeakPointer::FunctionToRegister)); 
  }



private: 

  ObjectType *m_Pointer;
    
  void FunctionToRegister () {
    m_Pointer = 0;  
    } 
  
};

}