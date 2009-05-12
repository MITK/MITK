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

#ifndef __CHERRY_COMMAND_UTILS_H__
#define __CHERRY_COMMAND_UTILS_H__

#include <string>
#include <vector>
#include <sstream>

#include <cherryObject.h>

namespace cherry
{

/**
 * A class providing utility functions for the commands plug-in.
 */
class CommandUtils
{
public:

  /**
   * Compares two boolean values. <code>false</code> is considered to be
   * less than <code>true</code>.
   *
   * @param left
   *            The left value to compare.
   * @param right
   *            The right value to compare.
   * @return <code>-1</code> if <code>left</code> is <code>false</code>
   *         and <code>right</code> is <code>true</code>;<code>0</code>
   *         if they are equal; <code>1</code> if <code>left</code> is
   *         <code>true</code> and <code>right</code> is
   *         <code>false</code>
   */
  static int Compare(const bool left, const bool right);

  static int Compare(const std::string& left, const std::string& right);

  /**
   * Compares two objects that are not otherwise comparable. If neither object
   * is <code>null</code>, then the string representation of each object is
   * used.
   *
   * @param left
   *            The left value to compare. The string representation of this
   *            value must not be <code>null</code>.
   * @param right
   *            The right value to compare. The string representation of this
   *            value must not be <code>null</code>.
   * @return <code>-1</code> if <code>left</code> is <code>null</code>
   *         and <code>right</code> is not <code>null</code>;
   *         <code>0</code> if they are both <code>null</code>;
   *         <code>1</code> if <code>left</code> is not <code>null</code>
   *         and <code>right</code> is <code>null</code>. Otherwise, the
   *         result of
   *         <code>left.toString().compareTo(right.toString())</code>.
   */
  static int CompareObj(const Object::ConstPointer left,
      const Object::ConstPointer right);

  /**
   * Tests whether two arrays of objects are equal to each other. The arrays
   * must not be <code>null</code>, but their elements may be
   * <code>null</code>.
   *
   * @param leftArray
   *            The left array to compare; may be <code>null</code>, and
   *            may be empty and may contain <code>null</code> elements.
   * @param rightArray
   *            The right array to compare; may be <code>null</code>, and
   *            may be empty and may contain <code>null</code> elements.
   * @return <code>true</code> if the arrays are equal length and the
   *         elements at the same position are equal; <code>false</code>
   *         otherwise.
   */
  template<class T>
  static int Compare(const std::vector<T>& leftArray,
      const std::vector<T>& rightArray) {
    int result = leftArray.size() - rightArray.size();
    if (result == 0)
    {
      for (unsigned int i = 0; i < leftArray.size(); ++i) {
        long int diff = (&(leftArray[i]) - &(rightArray[i]));
        int result = diff ? (diff < 0 ? -1 : 1) : 0;
        if (result != 0) break;
      }
    }

    return result;
  }

  /**
   * Tests whether two arrays of objects are equal to each other. The arrays
   * must not be <code>null</code>, but their elements may be
   * <code>null</code>.
   *
   * @param leftArray
   *            The left array to compare; may be <code>null</code>, and
   *            may be empty and may contain <code>null</code> elements.
   * @param rightArray
   *            The right array to compare; may be <code>null</code>, and
   *            may be empty and may contain <code>null</code> elements.
   * @return <code>true</code> if the arrays are equal length and the
   *         elements at the same position are equal; <code>false</code>
   *         otherwise.
   */
  template<class T>
  static bool Equals(const std::vector<T>& leftArray,
      const std::vector<T>& rightArray) {
    if (leftArray.size() != rightArray.size()) {
      return false;
    }

    for (unsigned int i = 0; i < leftArray.size(); i++) {
      T left = leftArray[i];
      T right = rightArray[i];
      const bool equal = left ? !right : (left == right);
      if (!equal) {
        return false;
      }
    }

    return true;
  }

  template<class T>
  static std::string ToString(const std::vector<T>& vec)
  {
    std::stringstream str;
    str << "[";
    for (unsigned int i = 0; i < vec.size(); ++i)
    {
      if (i > 0) str << ",";
      str << &(vec[i]);
    }
    return str.str();
  }
};

}

#endif // __CHERRY_COMMAND_UTILS_H__
