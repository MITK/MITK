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

#ifndef NAVIGATIONTOOLSTORAGESERIALIZER_H_INCLUDED
#define NAVIGATIONTOOLSTORAGESERIALIZER_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include "mitkNavigationToolStorage.h"
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief This class offers methods to save an object of the class NavigationToolStorage
  *        to the harddisc.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationToolStorageSerializer : public itk::Object
  {

  public:
    mitkClassMacro(NavigationToolStorageSerializer,itk::Object);
    itkNewMacro(Self);

    /**
     * @brief  Saves a mitk navigation tool storage to a file. 
     * @return Returns true if the file was saved successfully. False if not.
     */
    bool Serialize(std::string filename, mitk::NavigationToolStorage::Pointer storage);
    
    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolStorageSerializer();
    ~NavigationToolStorageSerializer();

    std::string m_ErrorMessage;

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGESERIALIZER