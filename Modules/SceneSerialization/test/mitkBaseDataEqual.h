/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkBaseDataEqual_h_included
#define mitkBaseDataEqual_h_included

#include "mitkBaseData.h"

namespace mitk
{

/**
  Interface to compare two BaseData objects for equality.
*/
class BaseDataEqual
{
protected:

  /// Gets to compare only non-nullptr objects
  virtual bool InternalAreEqual(const BaseData& left, const BaseData& right, double eps = mitk::eps, bool verbose = false) = 0;

  bool AreSameClasses(const BaseData* left, const BaseData* right, bool verbose = false);

public:

  bool AreEqual(const BaseData* left, const BaseData* right, double eps = mitk::eps, bool verbose = false);

  static void RegisterCoreEquals();
};

template <class T>
class BaseDataEqualT : public BaseDataEqual
{
  virtual bool InternalAreEqual(const BaseData& left, const BaseData& right, double eps = mitk::eps, bool verbose = false)
  {
    const T& leftT = dynamic_cast<const T&>(left);
    const T& rightT = dynamic_cast<const T&>(right);

    // mitk::Equal(Surface&) implemented as non-const
    T& leftNonConstT = const_cast<T&>(leftT);
    T& rightNonConstT = const_cast<T&>(rightT);

    return mitk::Equal(leftNonConstT, rightNonConstT, eps, verbose);
  }
};

} // namespace

#endif
