/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelFitStaticParameterMap_h
#define mitkModelFitStaticParameterMap_h

#include <vector>
#include <string>
#include <map>

#include <MitkModelFitExports.h>
#include <mitkModelBase.h>

namespace mitk
{
  namespace modelFit
  {
    /**
     * \class StaticParameterMap
     * \brief Data structure that stores information about the static parameters of a model fit.
     */
    class MITKMODELFIT_EXPORT StaticParameterMap
    {
    private:
      /** \brief Type of the map key. */
      typedef ModelBase::ParameterNameType KeyType;

    public:
      /** \brief Type of the map value. */
      typedef ModelBase::StaticParameterValuesType ValueType;
      /** \brief Type of a variable, consisting of name and value list. */
      typedef std::pair<KeyType, ValueType> StaticParameterType;

    private:
      /** \brief Type of the map. */
      typedef ModelBase::StaticParameterMapType MapType;

      /** \brief Stores the variables with their value lists. */
      MapType m_map;

      /**
       * \brief Stores the number of values that each list (with more than one value) contains.
       */
      unsigned int m_numValues;

    public:
      /** \brief Needed for 'foreach' support. */
      typedef MapType::const_iterator const_iterator;

      StaticParameterMap() : m_numValues(1) {}

      /**
       * \brief Adds the given value list under the given variable name.
       *
       * If the name already exists, nothing is added.
       *
       * \param name The name of the variable to which the values should be added.
       * \param newList The value list that should be added.
       * \pre The given list must contain either 1 or n values, where n is the
       * number of values in lists that are already part of the map (and contain
       * more than one value).
       * \throw ModelFitException If the list size is greater than 1 and doesn't match
       * the size of existing multi-value lists.
       */
      void Add(const std::string& name, const ValueType& newList);

      /**
       * \brief Returns the values of the given variable name.
       * \param name The name of the variable whose values should be returned.
       * \return The values of the given variable name.
       * \throw std::range_error If the variable name doesn't exist.
       */
      const ValueType& Get(const std::string& name) const;

      MapType::size_type Size() const
      {
        return m_map.size();
      }

      const_iterator begin() const
      {
        return m_map.begin();
      }

      const_iterator end() const
      {
        return m_map.end();
      }

      /**
       * \brief Sorts the map by the values of the given variable in ascending order.
       *
       * All other variables' values are reordered accordingly. If name is empty or
       * the variable could not be found, the map is ordered by the first variable
       * that contains more than one value (also in ascending order).
       *
       * \param name The name of the variable the map should be sorted by.
       */
      void Sort(const std::string& name = "");

      /**
       * \brief Resets the map so it's empty.
       */
      void Clear();
    };

    /**
     * \brief Compares two value lists by their first element.
     * \param a The first list to compare.
     * \param b The second list to compare.
     * \return True if the first list's first item is smaller than the second one's.
     */
    inline bool operator<(const StaticParameterMap::ValueType& a, const StaticParameterMap::ValueType& b)
    {
      return (a.front() < b.front());
    }
  }
}

#endif
