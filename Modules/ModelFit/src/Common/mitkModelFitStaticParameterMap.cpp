/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <algorithm>

#include "mitkModelFitStaticParameterMap.h"
#include "mitkModelFitException.h"

void mitk::modelFit::StaticParameterMap::Add(const std::string& name, const ValueType& newList)
{
	MapType::const_iterator mapIter = m_map.find(name);

	// Only add if the name doesn't exist yet
	if (mapIter == m_map.end())
	{
		// If the new list contains more than one values, make sure it contains the same number of
		// values as other lists that contain more than one value
		if (newList.size() > 1)
		{
			if (m_numValues > 1 && newList.size() != m_numValues)
			{
				mitkThrowException(ModelFitException) << "Unable to add variable '" << name
													  << "' to StaticParameterMap: It contains "
													  << newList.size() << "values (should be "
													  << m_numValues;
			}
			else if (m_numValues == 1)
			{
				m_numValues = newList.size();
			}
		}

		m_map.insert(StaticParameterType(name, newList));
	}
}

const mitk::modelFit::StaticParameterMap::ValueType&
mitk::modelFit::StaticParameterMap::Get(const std::string& name) const
{
	MapType::const_iterator mapIter = m_map.find(name);

	if (mapIter != m_map.end())
	{
		return mapIter->second;
	}

	throw std::range_error("'" + name + "' does not exist in InputStaticParameterMap");
}

void mitk::modelFit::StaticParameterMap::Sort(const std::string& name /* = "" */)
{
	if (m_map.empty() || m_numValues == 1)
	{
		return;
	}

	// Copy the actual map and operate on the copy to fulfill exception guarantees.
	MapType localMap = m_map;

	// Copy map contents to a m*n matrix where n is the number of values and m is the number of
	// variables
	std::vector<ValueType> temp;

	for (size_t i = 0; i < m_numValues; ++i)
	{
		temp.push_back(ValueType());
	}

	{
		MapType::const_iterator foundPair = localMap.find(name);

		if (!name.empty())
		{
			if (foundPair != localMap.end())
			{
				// An existing variable name has been given, so make sure its values are in the
				// first row of the matrix
				const ValueType& list = foundPair->second;

				if (list.size() > 1) // only copy lists that have more than one value
				{
					for (size_t i = 0; i < list.size(); ++i)
					{
						temp[i].push_back(list[i]);
					}
				}
			}
		}

		// Copy the rest of the values to the matrix
		for (MapType::const_iterator mapIter = localMap.begin(); mapIter != localMap.end();
			 ++mapIter)
		{
			if (!name.empty() && name == mapIter->first)
			{
				// Skip this variable since it was already added (in the first row)
				continue;
			}

			const ValueType& list = mapIter->second;

			if (list.size() > 1) // only copy lists that have more than one value
			{
				for (size_t i = 0; i < list.size(); ++i)
				{
					temp[i].push_back(list[i]);
				}
			}
		}
	}

	// Sort the matrix
	std::sort(temp.begin(), temp.end());

	// Copy the matrix contents back to the map
	{
		int i = 0;
		MapType::iterator foundPair = localMap.find(name);

		if (foundPair != localMap.end())
		{
			// An existing variable name has been given, so make sure the first row of the matrix is
			// copied back to that variable
			ValueType& list = foundPair->second;

			if (list.size() > 1) // only copy lists that have more than one value
			{
				list.clear();

				for (size_t j = 0; j < temp.size(); ++j)
				{
					list.push_back(temp[j][i]);
				}

				++i;
			}
		}

		// Copy the rest of the matrix back to the map
		for (MapType::iterator mapIter = localMap.begin(); mapIter != localMap.end(); ++mapIter)
		{
			if (!name.empty() && name == mapIter->first)
			{
				// Skip this variable since it was already filled (from the first row)
				continue;
			}

			ValueType& list = mapIter->second;

			if (list.size() > 1) // only copy lists that have more than one value
			{
				list.clear();

				for (size_t j = 0; j < temp.size(); ++j)
				{
					list.push_back(temp[j][i]);
				}

				++i;
			}
		}
	}

	// Write the now sorted copy back to the actual map
	m_map = localMap;
}

void mitk::modelFit::StaticParameterMap::Clear()
{
	m_map.clear();
	m_numValues = 1;
}
