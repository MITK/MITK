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

#ifndef mbilog_EXPORTS_H
#define mbilog_EXPORTS_H

  #ifndef MBILOG_MODULENAME
    #if defined(_CMAKE_MODULENAME)
      #define MBILOG_MODULENAME _CMAKE_MODULENAME
    #else
      #define MBILOG_MODULENAME "n/a"
    #endif
  #endif

  /**
   * provide a macro for adding compiler specific export/import declarations
   * to classes.
   *  This is needed for the export of symbols, when you build a DLL. Then write
   *
   *    class MitkIGT_EXPORT ClassName : public SomeClass {};
   */
  #if defined(_WIN32)
    #ifdef mbilog_EXPORTS
      #define MBILOG_DLL_API __declspec(dllexport)
    #else
      #define MBILOG_DLL_API __declspec(dllimport)
    #endif
  #else
    #define MBILOG_DLL_API
  #endif

#endif

