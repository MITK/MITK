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

#ifndef NAVIGATIONTOOLWRITER_H_INCLUDED
#define NAVIGATIONTOOLWRITER_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>

//mitk headers
#include <mitkCommon.h>
#include "mitkNavigationTool.h"
#include "mitkNavigationToolStorageSerializer.h"
#include <MitkIGTExports.h>


namespace mitk
{
  /**Documentation
  * \brief This class offers methods to write objects of the class navigation tool permanently
  *        to the harddisk. The objects are saved in a special fileformat which can be read
  *        by the class NavigationToolReader to restore the object.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NavigationToolWriter : public itk::Object
  {
  friend class mitk::NavigationToolStorageSerializer;

  public:
    mitkClassMacro(NavigationToolWriter,itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * @brief           Writes a navigation tool to a file.
     * @param  FileName The filename (complete, with path, C:/temp/myTool.igtTool for example)
     * @param  Tool     The tool which should be written to the file.
     * @return          Returns true if the file was written successfully, false if not. In the second
     *                  case you can get the error message by using the method GetErrorMessage().
     */
    bool DoWrite(std::string FileName,mitk::NavigationTool::Pointer Tool);

    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolWriter();
    ~NavigationToolWriter();
    std::string m_ErrorMessage;
    mitk::DataNode::Pointer ConvertToDataNode(mitk::NavigationTool::Pointer Tool);
    std::string GetFileWithoutPath(std::string FileWithPath);
    std::string ConvertPointSetToString(mitk::PointSet::Pointer pointSet);
    std::string ConvertPointToString(mitk::Point3D point);
    std::string ConvertQuaternionToString(mitk::Quaternion quat);
  };
} // namespace mitk
#endif //NAVIGATIONTOOLWRITER
