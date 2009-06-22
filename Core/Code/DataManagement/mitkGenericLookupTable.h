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

    typedef T ValueType;
    typedef std::map< unsigned int, ValueType > LookupTableType;

    mitkClassMacro(GenericLookupTable, itk::DataObject);
    mitkNewMacro1Param(GenericLookupTable<T>, T);
    itkNewMacro(Self);
    
    virtual ~GenericLookupTable() 
    {
    }

    void SetTableValue( unsigned int id, ValueType value )
    {
      m_LookupTable[id] = value;
    }

    ValueType &GetTableValue( unsigned int id )
    {
      return m_LookupTable[id];
    }

    LookupTableType *GetLookupTable()
    {
      return m_LookupTable;
    }


    bool operator==( const Self& lookupTable ) const
    {
      if ( m_LookupTable == lookupTable.GetLookupTable() )
      {
        return true;
      }
      else
      {
        return false;
      }
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
        
    LookupTableType m_LookupTable;
};

} // namespace mitk


#endif
