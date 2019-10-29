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

#ifndef mitkModelFitStaticParameterMap_h
#define mitkModelFitStaticParameterMap_h

#include <vector>
#include <string>
#include <map>

#include "MitkModelFitExports.h"
#include "mitkModelBase.h"

namespace mitk
{
  namespace modelFit
  {
    /** Data structure that is used to store information about the static parameters of a model fit*/
    class MITKMODELFIT_EXPORT StaticParameterMap
    {
    private:
      /**	@brief	Type of the map key */
      typedef ModelBase::ParameterNameType KeyType;

    public:
      /** @brief Type of the map value */
      typedef ModelBase::StaticParameterValuesType ValueType;
      /** @brief Type of a variable, consisting of name and value list */
      typedef std::pair<KeyType, ValueType> StaticParameterType;

    private:
      /** @brief Type of the map */
      typedef ModelBase::StaticParameterMapType MapType;

      /*	@brief Stores the variables with their value lists */
      MapType m_map;

      /**
      *	@brief	Stores the number of values that each list (which contains more than one
      *			value) contains.
      */
      unsigned int m_numValues;

    public:
      /** @brief Needed for 'foreach' support */
      typedef MapType::const_iterator const_iterator;

      StaticParameterMap() : m_numValues(1) {}

      /**
      *	@brief			Adds the given value list under the given variable name if the name
      *					doesn't exist already. If it does exist, nothing is added.
      *	@pre			The given list must contain either 1 or n values, where n is the
      *					amount of values that lists contain which are already part of the
      *					map (and contain more than one value). That means if the map is
      *					empty or contains only lists that have only one value each, this
      *					rule doesn't apply. An exception is thrown otherwise.
      *	@param name		The name of the variable to which the values should be added.
      *	@param newList	The value list that should be added.
      *	@throw ModelFitException	If the given list contains an amount of values that is
      *								greater than 1 and doesn't match the amount of values
      *								of the lists that are already part of the map (see
      *								pre-condition).
      */
      void Add(const std::string& name, const ValueType& newList);

      /**
      *	@brief		Returns the values of the given variable name.
      *	@param name	The name of the variables whose values should be returned.
      *	@return		The values of the given variable name.
      *	@throw std::range_error If the variable name doesn't exist.
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
      *	@brief		Sorts the values of the given variable name in ascending order. The
      *				values of all other variables will also be switched in that specific
      *				order. If name is empty or the variable could not be found, the map is
      *				ordered by the first variable that contains more than one value (also in
      *				ascending order).
      *	@details	Example:
      *				<li>Before sorting:
      *					"A": [3, 2, 5, 1, 4]
      *					"B": [0]
      *					"C": [3, 4, 1, 5, 2]
      *				<li>Sort():
      *					"A": [1, 2, 3, 4, 5]
      *					"B": [0]
      *					"C": [5, 4, 3, 2, 1]
      *				<li>Sort("B"):
      *					"A": [5, 4, 3, 2, 1]
      *					"B": [0]
      *					"C": [1, 2, 3, 4, 5]
      *				<li>Sort("X"):
      *					"A": [1, 2, 3, 4, 5]
      *					"B": [0]
      *					"C": [5, 4, 3, 2, 1]
      *	@param name	The name of the variable the map should be sorted by.
      */
      void Sort(const std::string& name = "");

      /**
      *	@brief	Resets the map, so it's empty.
      */
      void Clear();
    };

    /**
    *	@brief		Compares two var lists and returns true if the first list's first item is
    *				lower than the second one's.
    *	@param a	The first list to compare the other one to.
    *	@param b	The other list to compare the first one to.
    *	@return		True if the first list's first item is smaller than the second one's.
    */
    inline bool operator<(const StaticParameterMap::ValueType& a, const StaticParameterMap::ValueType& b)
    {
      return (a.front() < b.front());
    }
  }
}

#endif // mitkModelFitStaticParameterMap_h
