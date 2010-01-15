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


#ifndef CHERRYPAIR_H_
#define CHERRYPAIR_H_

namespace cherry {
namespace osgi {
namespace internal {

template<class T1, class T2>
struct Pair {
  typedef T1 first_type;
  typedef T2 second_type;

  T1 first;
  T2 second;

  Pair() : first(T1()), second(T2()) { }
  Pair(const T1& x, const T2& y) : first(x), second(y) { }
  template<class U, class V>
  Pair(const Pair<U,V>& p) : first(p.first), second(p.second) { }

  inline bool operator<(const Pair<T1,T2>& p) const
  {
    if (first) {
      return (first < p.first ? true : (first > p.first ? false : (second ? second < p.second : p.second)));
    }
    else {
      return (second ? second < p.second : p.second);
    }
  }

  inline bool operator>(const Pair<T1,T2>& p) const
  {
    if (first) {
      return (first > p.first ? true : (first < p.first ? false : (second ? second > p.second : !p.second)));
    }
    else {
      return (second ? second > p.second : !p.second);
    }
  }

  inline bool operator==(const Pair<T1,T2>& p) const
  {
    return (first ? first == p.first && (second ? second == p.second : !p.second) : (second ? second == p.second : !p.second));
  }

};

template<class T1, class T2>
Pair<T1,T2> MakePair(const T1& x, const T2& y)
{
  return Pair<T1,T2>(x, y);
}

template<class T1, class T2>
struct PairHash
  {
    inline std::size_t operator()(const Pair<T1,T2>* p) const
    {
      return ((p->first ? p->first->HashCode() : 0) ^
             (p->second ? p->second->HashCode() : 0));
    }
  };

}
}
}


#endif /* CHERRYPAIR_H_ */
