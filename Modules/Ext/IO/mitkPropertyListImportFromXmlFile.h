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
#ifndef mitkPropertyListImportFromXmlFile_h
#define mitkPropertyListImportFromXmlFile_h

#include <mitkPropertyList.h>
#include "MitkExtExports.h"

namespace mitk
{
 ///
 /// d pointer forward declaration
 ///
 struct PropertyListImportFromXmlFileData;
 ///
 /// DOCUMENTATION
 ///
 class MitkExt_EXPORT PropertyListImportFromXmlFile
 {
 public:
   ///
   /// init default values and save references
   ///
   PropertyListImportFromXmlFile( const std::string* _FileName = 0,
     PropertyList* _PropertyList = 0 );

   ///
   /// executes the algorithm if inputs changed
   ///
   void Update();
   ///
   /// delete d pointer
   ///
   virtual ~PropertyListImportFromXmlFile();
    ///
    /// setter for field FileName
    ///
    void SetFileName(const std::string* _FileName);
    ///
    /// setter for field PropertyList
    ///
    void SetPropertyList(PropertyList* _PropertyList);
 private:
   ///
   /// d pointer
   ///
   PropertyListImportFromXmlFileData* d;
 };
} // namespace mitk

#endif // mitkPropertyListImportFromXmlFile_h
