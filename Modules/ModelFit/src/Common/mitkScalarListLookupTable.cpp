/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkScalarListLookupTable.h"
#include <stdexcept>
#include <iostream>
const char* mitk::ScalarListLookupTable::GetNameOfClass() const
{
	return "ScalarListLookupTable";
}

void mitk::ScalarListLookupTable::SetTableValue(const KeyType& key, const ValueType& value)
{
	m_LookupTable[key] = value;
}

bool mitk::ScalarListLookupTable::ValueExists(const KeyType& key) const
{
	LookupTableType::const_iterator it = m_LookupTable.find(key);
	return (it != m_LookupTable.end());
}

const mitk::ScalarListLookupTable::ValueType&
mitk::ScalarListLookupTable::GetTableValue(const KeyType& key) const
{
	LookupTableType::const_iterator it = m_LookupTable.find(key);

	if (it != m_LookupTable.end())
	{
		return it->second;
	}
	else
	{
		throw std::range_error("id does not exist in the lookup table");
	}
}

const mitk::ScalarListLookupTable::LookupTableType&
mitk::ScalarListLookupTable::GetLookupTable() const
{
	return m_LookupTable;
}

void mitk::ScalarListLookupTable::SetLookupTable(const LookupTableType& table)
{
	m_LookupTable = table;
};


bool mitk::ScalarListLookupTable::operator==(const mitk::ScalarListLookupTable& lookupTable) const
{
	return (m_LookupTable == lookupTable.m_LookupTable);
}
bool mitk::ScalarListLookupTable::operator!=(const mitk::ScalarListLookupTable& lookupTable) const
{
	return !(m_LookupTable == lookupTable.m_LookupTable);
}

mitk::ScalarListLookupTable&
mitk::ScalarListLookupTable::operator=(const ScalarListLookupTable& other)
{
	if (this == &other)
	{
		return *this;
	}
	else
	{
		m_LookupTable = other.m_LookupTable;
		return *this;
	}
}

std::ostream& mitk::operator<<(std::ostream& stream, const ScalarListLookupTable& l)
{
	typedef ScalarListLookupTable::LookupTableType::const_iterator MapIterType;
	typedef ScalarListLookupTable::ValueType::const_iterator VectorIterType;
	MapIterType mapStart = l.GetLookupTable().begin();
	MapIterType mapEnd = l.GetLookupTable().end();
	stream << "[";

	for (MapIterType i = mapStart; i != mapEnd; ++i)
	{
		if (i != mapStart)
		{
			stream << ", ";
		}

		stream << i->first << " -> [";

		VectorIterType vectorStart = i->second.begin();
		VectorIterType vectorEnd = i->second.end();

		for (VectorIterType j = vectorStart; j != vectorEnd; ++j)
		{
			if (j != vectorStart)
			{
				stream << ", ";
			}

			stream << *j;
		}

		stream << "]";
	}

	return stream << "]";
};
