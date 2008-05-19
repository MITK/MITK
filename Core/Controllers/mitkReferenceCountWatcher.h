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

#include "itkCommand.h"

namespace mitk 
{

//##Documentation
//## @brief Keeps track of the reference count of an object even if
//## it is destroyed.
//## 
//## Example usage:
//## \code
//##   SomeFilter* filter = GetSomeFilter();
//##   ReferenceCountWatcher::Pointer filterWatcher;
//##   filterWatcher = new ReferenceCountWatcher(filter, "name of filter");
//##   filterWatcher->GetReferenceCount();
//## \endcode
//## @ingroup Testing
class ReferenceCountWatcher : public itk::Object
{
public:
  typedef itk::SimpleMemberCommand<ReferenceCountWatcher> CommandType;

  mitkClassMacro(ReferenceCountWatcher, itk::Object);

protected:
  //##Documentation
  //## @brief Object to be watched
  itk::Object* m_Object;

  //##Documentation
  //## @brief Optional comment, e.g. for debugging output
  std::string m_Comment;

  //##Documentation
  //## @brief If \a true, \a m_Object is no longer valid
  //## and the returned reference count will be 0.
  bool m_Deleted;

  //##Documentation
  //## @brief itk::Command to get a notification when the object 
  //## is deleted.
  CommandType::Pointer m_DeleteCommand;

public:
  //##Documentation
  //## @brief Constructor requiring object to be watched and allowing
  //## an optional comment.
  ReferenceCountWatcher(itk::Object* o, const char *comment="") : m_Object(o), m_Comment(comment), m_Deleted(false), m_ObserverTag(0)
  {
    m_DeleteCommand = CommandType::New();
    m_DeleteCommand->SetCallbackFunction(this, &ReferenceCountWatcher::DeleteObserver);
    if(m_Object!=NULL)
      m_ObserverTag = m_Object->AddObserver(itk::DeleteEvent(), m_DeleteCommand);
    m_ReferenceCountLock.Lock();
    m_ReferenceCount = 0;
    m_ReferenceCountLock.Unlock();
  }
  //##Documentation
  //## @brief Destructor: remove observer
  ~ReferenceCountWatcher()
  {
    if((m_Deleted == false) && (m_Object != NULL))
    {
      m_Object->RemoveObserver(m_ObserverTag);
    }
  }
  //##Documentation
  //## @brief Return the reference count of the watched object or
  //## 0 if it has been destroyed
  int GetReferenceCount() const
  {
    if(m_Object == NULL) return -1;
    if(m_Deleted) return 0;
    return m_Object->GetReferenceCount();
  }

  //##Documentation
  //## @brief Return the optional string comment
  itkGetStringMacro(Comment);
protected:
  //##Documentation
  //## @brief Callback called on itk::DeleteEvent() of wathched object.
  void DeleteObserver()
  {
    m_Deleted = true;
  }
  unsigned long m_ObserverTag;
};

}
