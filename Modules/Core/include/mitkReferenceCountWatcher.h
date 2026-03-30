/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkReferenceCountWatcher_h
#define mitkReferenceCountWatcher_h

#include <itkCommand.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  /**
   * \brief Keeps track of the reference count of an object even if it is destroyed.
   *
   * This utility class registers itself as an ITK observer for the DeleteEvent
   * of the watched object. After the object is destroyed, GetReferenceCount()
   * returns 0 instead of accessing invalid memory.
   *
   * \ingroup Testing
   */
  class ReferenceCountWatcher : public itk::Object
  {
  public:
    typedef itk::SimpleMemberCommand<ReferenceCountWatcher> CommandType;

    mitkClassMacroItkParent(ReferenceCountWatcher, itk::Object);

  protected:
    /** \brief The object to be watched. */
    itk::Object *m_Object;

    /** \brief Optional comment, e.g. for debugging output. */
    std::string m_Comment;

    /** \brief If true, m_Object is no longer valid and the returned reference count will be 0. */
    bool m_Deleted;

    /** \brief itk::Command to get a notification when the object is deleted. */
    CommandType::Pointer m_DeleteCommand;

  public:
    /** \brief Constructor requiring the object to be watched and allowing an optional comment.
     *
     * \param o the ITK object whose reference count should be tracked.
     * \param comment an optional descriptive string for debugging.
     */
    ReferenceCountWatcher(itk::Object *o, const char *comment = "")
      : m_Object(o), m_Comment(comment), m_Deleted(false), m_ObserverTag(0)
    {
      m_DeleteCommand = CommandType::New();
      m_DeleteCommand->SetCallbackFunction(this, &ReferenceCountWatcher::DeleteObserver);
      if (m_Object != nullptr)
        m_ObserverTag = m_Object->AddObserver(itk::DeleteEvent(), m_DeleteCommand);
      m_ReferenceCount = 0;
    }

    /** \brief Destructor. Removes the observer from the watched object if it still exists. */
    ~ReferenceCountWatcher() override
    {
      if ((m_Deleted == false) && (m_Object != nullptr))
      {
        m_Object->RemoveObserver(m_ObserverTag);
      }
    }

    /** \brief Returns the reference count of the watched object.
     *
     * \return The reference count, 0 if the object has been destroyed,
     *         or -1 if no object was provided.
     */
    int GetReferenceCount() const override
    {
      if (m_Object == nullptr)
        return -1;
      if (m_Deleted)
        return 0;
      return m_Object->GetReferenceCount();
    }

    /** \brief Returns the optional string comment. */
    itkGetStringMacro(Comment);

  protected:
    /** \brief Callback invoked on itk::DeleteEvent() of the watched object. */
    void DeleteObserver() { m_Deleted = true; }
    unsigned long m_ObserverTag;  ///< ITK observer tag for the delete event.
  };
}

#endif
