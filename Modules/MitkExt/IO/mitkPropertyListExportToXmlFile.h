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
#ifndef mitkPropertyListExportToXmlFile_h
#define mitkPropertyListExportToXmlFile_h

#include "MitkExtExports.h"
#include <mitkPropertyList.h>

namespace mitk
{
 ///
 /// d pointer forward declaration
 ///
 struct PropertyListExportToXmlFileData;
 ///
 /// writes a 2d cv point to an xml file
 ///
 class MitkExt_EXPORT PropertyListExportToXmlFile
 {
 public:
   ///
   /// init default values and save references
   ///
   PropertyListExportToXmlFile( const std::string* _FileName = 0,
     const PropertyList* _PropertyList = 0);
   ///
   /// executes the algorithm if inputs changed
   ///
   void Update();
   ///
   /// delete d pointer
   ///
   virtual ~PropertyListExportToXmlFile();
    ///
    /// setter for field FileName
    ///
    void SetFileName(const std::string* _FileName);
    ///
    /// setter for field PropertyList
    ///
    void SetPropertyList(const PropertyList* _PropertyList);
 private:
   ///
   /// d pointer
   ///
   PropertyListExportToXmlFileData* d;
 };
} // namespace mitk

#endif // mitkPropertyListExportToXmlFile_h
