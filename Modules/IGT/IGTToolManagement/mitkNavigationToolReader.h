/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef NAVIGATIONTOOLREADER_H_INCLUDED
#define NAVIGATIONTOOLREADER_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include "mitkNavigationTool.h"
#include "mitkDataStorage.h"
#include "mitkNavigationToolStorageDeserializer.h"
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief This class offers methods to read objects of the class NavigationTool from the
  *        harddisc. The tools have to be saved in a special format by the class NavigationToolWriter
  *        to be loadable.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationToolReader : public itk::Object
  {
  friend class mitk::NavigationToolStorageDeserializer;
  public:
    mitkClassMacro(NavigationToolReader,itk::Object);
    mitkNewMacro1Param(Self,mitk::DataStorage::Pointer);

    /**
     * @brief          This method reads a navigation tool from a file.
     * @param filename The filename where the tool is stored, "C:\temp\myTool.igtTool" for example.
     * @return         Returns a pointer to the tool which was read. Returns NULL, if something went
     *                 wrong and no tool was read. In this case you may also want the error message which is availiable
     *                 from the method GetErrorMessage().
     */
    mitk::NavigationTool::Pointer DoRead(std::string filename);
    
    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolReader(mitk::DataStorage::Pointer dataStorage);
    ~NavigationToolReader();

    std::string m_ErrorMessage;

    mitk::DataStorage::Pointer m_DataStorage;

    mitk::NavigationTool::Pointer ConvertDataNodeToNavigationTool(mitk::DataNode::Pointer node, std::string toolPath);

    //################### protected help methods ########################
    std::string GetFileWithoutPath(std::string FileWithPath);

  };
} // namespace mitk
#endif //NAVIGATIONTOOLREADER