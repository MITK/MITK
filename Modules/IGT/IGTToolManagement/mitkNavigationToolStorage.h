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
#include "mitkNavigationTool.h"

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
    
    /**
     * @brief  Adds a tool to the storage. Be sure that the tool has a unique 
     *         identifier which is not already part of this storage.
     * @return Returns true if the tool was added to the storage, false if not
     *         (false can be returned if the identifier already exists in this storage
     *         for example).
     */
    bool AddTool(mitk::NavigationTool::Pointer tool);
    
    /**
     * @return Returns the tracking tool at the position "number"
     *         in the storage. Returns NULL if there is no
     *         tracking tool at this position.
     */
    mitk::NavigationTool::Pointer GetTool(int number);
    
    /**
     * @return Returns the tracking tool with the given identifier.
     *         Returns NULL if there is no
     *         tracking tool with this identifier in the storage.
     */
    mitk::NavigationTool::Pointer GetTool(std::string identifier);
    

    /**
     * @brief Deletes a tool from the collection.
     */
    bool DeleteTool(int number);

    /**
     * @return Returns the number of tools stored in the storage.
     */
    int GetToolCount();

    /**
     * @return Returns true if the storage is empty, false if not.
     */
    bool isEmpty();
    
  protected:
    NavigationToolStorage();
    ~NavigationToolStorage();
    
    std::vector<mitk::NavigationTool::Pointer> m_ToolCollection;

  };
} // namespace mitk
#endif //NAVIGATIONTOOLSTORAGE