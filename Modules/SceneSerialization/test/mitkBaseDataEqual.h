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
 * \brief Interface to compare two BaseData objects for (near) equality.
 *
 * Classes implementing this interface must be able to judge two
 * given instances of a certain BaseData type as equal or not.
 *
 * The definition of "equal" is left to individual classes! (original
 * use case being the comparison of BaseData objects after serialization)
 *
 *
 * This interface is used by DataStorageCompare to compare two given
 * BaseData instances. The interface is required to handle all data
 * types in an identical way (alternatives: operator== is too strict;
 * mitk::Equal() has different signatures).
 *
 * To re-use what already has been implemented in various mitk::Equal()
 * functions, this interface is accompanied by a template class
 * BaseDataEqualT which makes use of an existing mitk::Equal() method.
 *
 * \warning Current convention: service instances are described by a "basedata" property
 *          that contains the GetNameOfClass() string of the BaseData type that can be
 *          compared by this instance. mitk::DataStorageCompare uses this convention.
 *
 */
class BaseDataEqual
{
private:

  /**
   * \brief Method to override in sub-classes.
   *
   * Gets called by AreEqual() which already verifies that left and right are
   * valid objects (not nullptr) and are of the same type!
   */
  virtual bool InternalAreEqual(const BaseData& left, const BaseData& right, double eps = mitk::eps, bool verbose = false) = 0;

  /**
   * \brief Helper to AreEqual(), tests GetNameOfClass().
   */
  bool AreSameClasses(const BaseData* left, const BaseData* right, bool verbose = false);

public:

  /**
   * \brief Main interface method, compares two given BaseData instances.
   *
   * Compares left and right BaseData instance using a given epsilon/precision
   * for potential float value comparisons. Can print warnings when the verbose
   * flag is given.
   *
   * \param left left BaseData instance of the comparison.
   * \param right right BaseData instance of the comparison.
   * \param eps precision for float value comparisons.
   * \param verbose when true, failing comparisons will print messages to logging/console.
   */
  bool AreEqual(const BaseData* left, const BaseData* right, double eps = mitk::eps, bool verbose = false);

  /**
   * \brief Register core type comparators that come with mitk::Equal() functions.
   */
  static void RegisterCoreEquals();
};

/**
 * \brief Implementation of BaseDataEqual that uses mitk:Equal() for comparisons.
 *
 * See base class for details!
 *
 * \sa BaseDataEqual
 */
template <class T>
class BaseDataEqualT : public BaseDataEqual
{
private:

  virtual bool InternalAreEqual(const BaseData& left, const BaseData& right, double eps = mitk::eps, bool verbose = false)
  {
    try
    {
      const T& leftT = dynamic_cast<const T&>(left);
      const T& rightT = dynamic_cast<const T&>(right);

      // mitk::Equal(Surface&) implemented as non-const
      T& leftNonConstT = const_cast<T&>(leftT);
      T& rightNonConstT = const_cast<T&>(rightT);

      return mitk::Equal(leftNonConstT, rightNonConstT, eps, verbose);
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << "Bad cast in BaseDataEqualT<>::InternalAreEqual()";
      return false;
    }
  }
};

} // namespace

#endif
