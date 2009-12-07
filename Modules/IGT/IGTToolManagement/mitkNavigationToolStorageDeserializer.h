/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-08-11 15:15:02 +0200 (Di, 11 Aug 2009) $
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef NAVIGATIONTOOLSTORAGEDESERIALIZER_H_INCLUDED
#define NAVIGATIONTOOLSTORAGEDESERIALIZER_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief This class offers methods to load an object of the class NavigationToolStorage
  *        from the harddisc.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationToolStorageDeserializer : public itk::Object
  {
  public:
    mitkClassMacro(NavigationToolStorageDeserializer,itk::Object);
    itkNewMacro(Self);
    
  protected:
    NavigationToolStorageDeserializer();
    ~NavigationToolStorageDeserializer();

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGEDESERIALIZER