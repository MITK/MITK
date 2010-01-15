/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef CHERRYREFERENCECOUNTER_H_
#define CHERRYREFERENCECOUNTER_H_

#include <map>
#include <vector>

namespace cherry
{

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * A ReferenceCounter is used to reference counting objects.
 * Each object is identified by a unique ID.  Together they form
 * an ID - value pair. An object is added to the counter by calling
 * #put(id, object).  From this point on additional refs can be made
 * by calling #addRef(id) or #removeRef(id).
 */
template<class I, class V> class ReferenceCounter
{

  /**
   * Capture the information about an object.
   */
public:

  class RefRec
  {
public:
    RefRec() :
      m_RefCount(0)
    {
    }

    RefRec(I id, V value) : m_Id(id), m_Value(value), m_RefCount(0)
    {
      AddRef();
    }

    I GetId()
    {
      return m_Id;
    }

    V GetValue()
    {
      return m_Value;
    }

    int AddRef()
    {
      ++m_RefCount;
      return m_RefCount;
    }

    int RemoveRef()
    {
      --m_RefCount;
      return m_RefCount;
    }

    int GetRef()
    {
      return m_RefCount;
    }

    bool IsNotReferenced()
    {
      return (m_RefCount <= 0);
    }

    I m_Id;

    V m_Value;

  private:

    int m_RefCount;
  };
  
private:
  std::map<I, RefRec> mapIdToRec;
  
  
public:

  /**
   * Creates a new counter.
   */
  ReferenceCounter()
  {

  }

  /**
   * Adds one reference to an object in the counter.
   *
   * @param id is a unique ID for the object.
   * @return the new ref count
   */
  int AddRef(I id)
  {
    typename std::map<I, RefRec>::iterator rec = mapIdToRec.find(id);
    if (rec == mapIdToRec.end())
    {
      return 0;
    }
    return rec->second.AddRef();
  }

  /**
   * Returns the object defined by an ID.  If the ID is not
   * found <code>null</code> is returned.
   *
   * @return the object or <code>null</code>
   */
  V Get(I id)
  {
    typename std::map<I, RefRec>::iterator rec = mapIdToRec.find(id);
    if (rec == mapIdToRec.end())
    {
      return V();
    }
    return rec->second.GetValue();
  }

  /**
   * Returns a complete list of the keys in the counter.
   *
   * @return a Set containing the ID for each.
   */
  std::vector<I> KeyVector()
  {
    std::vector<I> keys;
    for (typename std::map<I, RefRec>::iterator iter = mapIdToRec.begin(); iter
        != mapIdToRec.end(); ++iter)
    {
      keys.push_back(iter->first);
    }
    return keys;
  }

  /**
   * Adds an object to the counter for counting and gives
   * it an initial ref count of 1.
   *
   * @param id is a unique ID for the object.
   * @param value is the object itself.
   */
  void Put(I id, V value)
  {
    RefRec rec(id, value);
    mapIdToRec[id] = rec;
  }

  /**
   * @param id is a unique ID for the object.
   * @return the current ref count
   */
  int GetRef(I id)
  {
    RefRec rec = mapIdToRec[id];
    return rec.GetRef();
  }

  /**
   * Removes one reference from an object in the counter.
   * If the ref count drops to 0 the object is removed from
   * the counter completely.
   *
   * @param id is a unique ID for the object.
   * @return the new ref count
   */
  int RemoveRef(I id)
  {
    RefRec rec = mapIdToRec[id];
    if (rec.GetRef() == 0)
    {
      return 0;
    }
    int newCount = rec.RemoveRef();
    if (newCount <= 0)
    {
      mapIdToRec.erase(id);
    }
    return newCount;
  }

  /**
   * Returns a complete list of the values in the counter.
   *
   * @return a Collection containing the values.
   */
  std::vector<V> Values()
  {

    std::vector<V> values;
    for (typename std::map<I, RefRec>::iterator iter = mapIdToRec.begin(); iter
        != mapIdToRec.end(); ++iter)
    {
      values.push_back(iter->second.GetValue());
    }
    return values;
  }
};

}

#endif /*CHERRYREFERENCECOUNTER_H_*/
