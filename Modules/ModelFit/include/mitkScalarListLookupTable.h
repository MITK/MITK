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

#ifndef mitkScalarListLookupTable_h
#define mitkScalarListLookupTable_h

#include <string>
#include <vector>
#include <map>

#include "MitkModelFitExports.h"

namespace mitk
{
    /**
     *  @brief  Data class for modelfit properties that store a map of lists (e.g. static
     *          parameters).
     */
    class MITKMODELFIT_EXPORT ScalarListLookupTable
    {
    public:
        typedef std::string KeyType;
        typedef std::vector<double> ValueType;
        typedef std::map<KeyType, ValueType> LookupTableType;
        typedef std::pair<KeyType, ValueType> EntryType;

        ScalarListLookupTable() {}
        virtual ~ScalarListLookupTable() {}

        virtual const char* GetNameOfClass() const;

        /**
         *  @brief          Sets the list at the given map key to the given value.
         *  @param key      The name of the list (i.e. map key).
         *  @param value    The list to be added (i.e. map value).
         */
        void SetTableValue(const KeyType& key, const ValueType& value);

        /**
         *  @brief      Returns true if the list with the given name exists.
         *  @param key  The name of the desired list (i.e. map key)
         *  @return     true if the list exists or false otherwise.
         */
        bool ValueExists(const KeyType& key) const;

        /**
         *  @brief      Returns the list with the given name.
         *  @param key  The name (i.e. map key) of the desired list.
         *  @return     The list with the given name.
         *  @throw std::range_error If the list doesn't exist.
         */
        const ValueType& GetTableValue(const KeyType& key) const;

        /**
         *  @brief  Returns the map of lists.
         *  @return The map of lists.
         */
        const LookupTableType& GetLookupTable() const;

        void SetLookupTable(const LookupTableType& table);

        bool operator==(const ScalarListLookupTable& lookupTable) const;
        bool operator!=(const ScalarListLookupTable& lookupTable) const;

        virtual ScalarListLookupTable& operator=(const ScalarListLookupTable& other);

    protected:
        LookupTableType m_LookupTable;
    };

    /**
     *  @brief          Adds the string representation of the given ScalarListLookupTable to the
     *                  given stream.
     *  @param stream   The stream to which the map's string representation should be added.
     *  @param l        The map whose string representation should be added to the stream.
     *  @return         The given stream with the added string representation of the map.
     */
    MITKMODELFIT_EXPORT std::ostream& operator<<(std::ostream& stream,
            const ScalarListLookupTable& l);
}

#endif // mitkScalarListLookupTable_h
