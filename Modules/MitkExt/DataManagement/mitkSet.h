#ifndef mitkSet_H
#define mitkSet_H

#include <itkDataObject.h>
#include <itkCommand.h>
#include <mitkCommon.h>
#include <vector>
#include <set>

#include "mitkSetObserver.h"

namespace mitk
{

///
/// A class that acts like a weak pointer
/// but for a list of itk objects
///
template <class T>
class Set: public itk::DataObject
{
public:
  typedef mitk::SetObserver<T> Observer;
  mitkClassMacro(Set<T>, itk::Object);
  itkFactorylessNewMacro(Set<T>);

  Set()
  {
  }

  bool Add ( const T& obj )
  {
    if(this->Has(obj))  // this is a set! do not add twice
      return false;

    // add it now
    m_Objects.push_back(obj); // if index is not valid any more, just add
                              // the element
    // subscribe for modified event
    typename itk::MemberCommand<mitk::Set<T> >::Pointer _modifiedCommand =
      itk::MemberCommand<mitk::Set<T> >::New();
    _modifiedCommand->SetCallbackFunction(this
      , &Set<T>::OnObjectModified);
    m_ObjectModifiedTags[obj] =
        obj->AddObserver(itk::ModifiedEvent(), _modifiedCommand);

    // subscribe for delete event
    typename itk::MemberCommand<mitk::Set<T> >::Pointer _DeleteCommand =
      itk::MemberCommand<mitk::Set<T> >::New();
    _DeleteCommand->SetCallbackFunction(this
      , &Set<T>::OnObjectModified);
    m_ObjectDeleteTags[obj] =
        obj->AddObserver(itk::DeleteEvent(), _DeleteCommand);

    for(typename std::set<SetObserver<T>*>::iterator it = m_SetObserver.begin();
      it != m_SetObserver.end(); ++it)
        (*it)->OnAdded(obj);

    this->Modified();
    return true;
  }

  bool Remove ( const T& obj )
  {
    return this->Remove(this->IndexOf(obj));
  }

  bool Remove ( int index )
  {
    if( !this->IsValid(index) ) // element must exist to be removed
      return false;

    typename std::vector<T>::iterator it = m_Objects.begin();
    std::advance(it, index);

    T& obj = *it;

    for(typename std::set<SetObserver<T>*>::iterator it2
        = m_SetObserver.begin(); it2 != m_SetObserver.end(); ++it2)
        (*it2)->OnRemove(*it);
    // remove it now
    obj->RemoveObserver(m_ObjectModifiedTags[obj]);
    obj->RemoveObserver(m_ObjectDeleteTags[obj]);
    m_ObjectModifiedTags.erase(obj);
    m_ObjectDeleteTags.erase(obj);
    m_Objects.erase(it);
    this->Modified();
    return true;
  }

  void Clear ()
  {
    while(m_Objects.size() > 0)
      this->Remove(m_Objects.size()-1);
  }

  int GetSize() const
  {
    return m_Objects.size();
  }
  int IndexOf(const T& obj) const
  {
    int index = -1;
    typename std::vector<T>::const_iterator it = m_Objects.begin();
    for(unsigned int i=0; i<m_Objects.size(); ++i)
    {
      if(m_Objects.at(i) == obj)
      {
        index = i;
        break;
      }
    }
    return index;
  }
  bool Has(const T& obj) const
  {
    return this->IndexOf(obj) != -1;
  }
  bool IsEmpty() const
  {
    return m_Objects.empty();
  }
  bool IsValid( unsigned int index ) const
  {
    return m_Objects.size() > 0 && index >=0 && index < m_Objects.size();
  }
  T& Front()
  {
    return m_Objects.front();
  }
  T& Back()
  {
    return m_Objects.back();
  }
  T& Get( unsigned int index )
  {
    return m_Objects.at(index);
  }
  const T& Front() const
  {
    return m_Objects.front();
  }
  const T& Back() const
  {
    return m_Objects.back();
  }
  const T& Get( unsigned int index ) const
  {
    return m_Objects.at(index);
  }
  void AddObserver( SetObserver<T>* observer ) const
  {
    m_SetObserver.insert( observer );
  }
  void RemoveObserver( SetObserver<T>* observer ) const
  {
    m_SetObserver.erase( observer );
  }
  void OnObjectModified(const itk::Object* caller
                                          , const itk::EventObject &event)
  {
    unsigned int i=0;
    for(; i<m_Objects.size(); ++i)
      if(m_Objects.at(i) == caller)
        break;

    const itk::DeleteEvent* delEvent
        = dynamic_cast<const itk::DeleteEvent*>(&event);

    // inform listeners
    for(typename std::set<SetObserver<T>*>::iterator it = m_SetObserver.begin();
      it != m_SetObserver.end(); ++it)
      delEvent ? (*it)->OnDelete( this->Get(i) )
        : (*it)->OnModified( this->Get(i) );

    // remove from list if object was deleted (no dangling pointers)
    if(delEvent)
    {
      this->Remove(i);
    }
  }

  Set(const Set<T>& other)
  {
    *this = other;
  }
  Set<T>& operator=
      (const Set<T>& other)
  {
    // do not simply copy -> because of observer objects
    // instead: use add method for each element of the other List
    for(int i=0; i<other.GetSize(); ++i)
      this->Add( other.Get(i) );
  }
  virtual ~Set()
  {
    this->Clear();
  }
protected:
  ///
  /// Holds all objects
  ///
  std::vector<T> m_Objects;

  ///
  /// holds the list of observed itk objects (will be updated in setat())
  ///
  mutable std::set<SetObserver<T>*> m_SetObserver;

  ///
  /// \brief Holds all tags of Modified Event Listeners.
  ///
  std::map<const T, unsigned long> m_ObjectModifiedTags;

  ///
  /// \brief Holds all tags of Modified Event Listeners.
  ///
  std::map<const T, unsigned long> m_ObjectDeleteTags;
};

} // namespace mitk

#endif // mitkSet_H
