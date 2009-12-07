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

#ifndef NAVIGATIONTOOLSTORAGE_H_INCLUDED
#define NAVIGATIONTOOLSTORAGE_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief An object of this class represents a collection of navigation tools.
  *        You may add/delete navigation tools or store/load the whole collection
  *        to/from the harddisc by using the class NavigationToolStorageSerializer
  *        and NavigationToolStorageDeserializer.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationToolStorage : public itk::Object
  {
  public:
    mitkClassMacro(NavigationToolStorage,itk::Object);
    itkNewMacro(Self);
    
  protected:
    NavigationToolStorage();
    ~NavigationToolStorage();

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGE