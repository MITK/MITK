/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __BERRY_COMMAND_UTILS_H__
#define __BERRY_COMMAND_UTILS_H__

#include <berryObject.h>

#include <QList>
#include <QTextStream>

namespace berry
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

  static int Compare(const QString& left, const QString& right);

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
  static int Compare(const QList<T>& leftArray,
                     const QList<T>& rightArray)
  {
    int result = (int) (leftArray.size() - rightArray.size());
    if (result == 0)
    {
      for (int i = 0; i < leftArray.size(); ++i)
      {
        if (!(leftArray[i] == rightArray[i]))
        {
          return leftArray[i]->ToString().compare(rightArray[i]->ToString());
        }
      }
    }

    return result < 0 ? -1 : (result > 0 ? 1 : 0);
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
  static bool Equals(const QList<T>& leftArray,
                     const QList<T>& rightArray)
  {
    return leftArray == rightArray;
  }

  template<class T>
  static QString ToString(const QList<T>& vec)
  {
    QString str;
    QTextStream ss(&str);
    ss << "[";
    for (int i = 0; i < vec.size(); ++i)
    {
      if (i > 0) ss << ",";
      ss << vec[i];
    }
    return str;
  }
};

}

#endif // __BERRY_COMMAND_UTILS_H__
