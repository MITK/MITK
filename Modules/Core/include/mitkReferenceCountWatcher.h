/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkCommand.h"
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  //##Documentation
  //## @brief Keeps track of the reference count of an object even if
  //## it is destroyed.
  //## @ingroup Testing
  class ReferenceCountWatcher : public itk::Object
  {
  public:
    typedef itk::SimpleMemberCommand<ReferenceCountWatcher> CommandType;

    mitkClassMacroItkParent(ReferenceCountWatcher, itk::Object);

  protected:
    //##Documentation
    //## @brief Object to be watched
    itk::Object *m_Object;

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
    ReferenceCountWatcher(itk::Object *o, const char *comment = "")
      : m_Object(o), m_Comment(comment), m_Deleted(false), m_ObserverTag(0)
    {
      m_DeleteCommand = CommandType::New();
      m_DeleteCommand->SetCallbackFunction(this, &ReferenceCountWatcher::DeleteObserver);
      if (m_Object != nullptr)
        m_ObserverTag = m_Object->AddObserver(itk::DeleteEvent(), m_DeleteCommand);
      m_ReferenceCount = 0;
    }
    //##Documentation
    //## @brief Destructor: remove observer
    ~ReferenceCountWatcher() override
    {
      if ((m_Deleted == false) && (m_Object != nullptr))
      {
        m_Object->RemoveObserver(m_ObserverTag);
      }
    }
    //##Documentation
    //## @brief Return the reference count of the watched object or
    //## 0 if it has been destroyed
    int GetReferenceCount() const override
    {
      if (m_Object == nullptr)
        return -1;
      if (m_Deleted)
        return 0;
      return m_Object->GetReferenceCount();
    }

    //##Documentation
    //## @brief Return the optional string comment
    itkGetStringMacro(Comment);

  protected:
    //##Documentation
    //## @brief Callback called on itk::DeleteEvent() of wathched object.
    void DeleteObserver() { m_Deleted = true; }
    unsigned long m_ObserverTag;
  };
}
