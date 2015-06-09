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
#ifndef mitkIPersistable_h
#define mitkIPersistable_h
#include <string>

namespace mitk
{
    ///
    /// Just a base class for persistable classes
    ///
    class MITKPERSISTENCE_EXPORT IPersistable
    {
    public:
        virtual bool Save(const std::string& fileName="") = 0;
        virtual bool Load(const std::string& fileName="") = 0;
        virtual void SetId(const std::string& id) = 0;
        virtual std::string GetId() const = 0;
    };
}

#endif
