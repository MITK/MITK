/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-06-24 19:37:48 +0200 (Di, 24 Jun 2008) $
Version:   $Revision: 14641 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGENERICLOOKUPTABLE_H_HEADER_INCLUDED_C1061CEE
#define MITKGENERICLOOKUPTABLE_H_HEADER_INCLUDED_C1061CEE

#include <string>
#include <sstream>
#include <stdlib.h>
#include <map>

#include <itkDataObject.h>

#include "mitkVector.h"
#include "mitkCommon.h"


namespace mitk {

/**
 * @brief Template class for generating lookup-tables
 * 
 * This class template can be instantiated for all classes/internal types that fulfills
 *  these requirements:
 * - an operator<< so that the properties value can be put into a std::stringstream
 * - an operator== so that two properties can be checked for equality
 *
 * The main purpose of this class is to be used in conjunction with
 * GenericLookupTableProperty. This enables passing of arbitrary lookup
 * tables to mappers to configure the rendering process.
 */
template <typename T>
class GenericLookupTable : public itk::DataObject
{
  public:
    typedef unsigned int IdentifierType;
    typedef T ValueType;
    typedef std::map< IdentifierType, ValueType > LookupTableType;

    mitkClassMacro(GenericLookupTable, itk::DataObject);
    itkNewMacro(Self);
    
    void SetTableValue( IdentifierType id, ValueType value )
    {
      m_LookupTable[id] = value;
    }

    bool ValueExists(IdentifierType id) const
    {
      typename LookupTableType::const_iterator it = m_LookupTable.find(id);
      return (it != m_LookupTable.end());
    }

    ValueType GetTableValue( IdentifierType id ) const
    {
      typename LookupTableType::const_iterator it = m_LookupTable.find(id);
      if (it != m_LookupTable.end())
        return it->second;
      else
        throw std::range_error("id does not exist in the lookup table");
    }

    //const LookupTableType& GetLookupTable() const
    //{
    //  return m_LookupTable;
    //}

    bool operator==( const Self& lookupTable ) const
    {
      return (m_LookupTable == lookupTable.m_LookupTable);
    }

    virtual Self& operator=(const Self& other)
    {
      if ( this == &other )
      {
        return *this;
      }
      else
      {
        //TODO: implement copying of values
        return *this;
      }
    }

  protected:
    GenericLookupTable() {}
    virtual ~GenericLookupTable() 
    {
    }

    LookupTableType m_LookupTable;
};
} // namespace mitk

/**
* Generates a specialized subclass of mitk::GenericLookupTable. 
* This way, GetNameOfClass() returns the value provided by LookupTableName.
* Please see mitkProperties.h for examples.
* @param LookupTableName the name of the instantiation of GenericLookupTable
* @param Type the value type of the GenericLookupTable
*/
#define mitkSpecializeGenericLookupTable(LookupTableName,Type)  \
class MITK_CORE_EXPORT LookupTableName: public GenericLookupTable< Type >    \
{                                                               \
public:                                                         \
  mitkClassMacro(LookupTableName, GenericLookupTable< Type >);  \
  itkNewMacro(LookupTableName);                                 \
protected:                                                      \
  LookupTableName() {}                                          \
  virtual ~LookupTableName() {}                                 \
};

#endif
