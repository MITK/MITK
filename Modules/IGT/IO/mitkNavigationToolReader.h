/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  class MITKIGT_EXPORT NavigationToolReader : public itk::Object
  {
  friend class mitk::NavigationToolStorageDeserializer;
  public:
    mitkClassMacroItkParent(NavigationToolReader,itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * @brief          This method reads a navigation tool from a file.
     * @param filename The filename where the tool is stored, "C:\temp\myTool.igtTool" for example.
     * @return         Returns a pointer to the tool which was read. Returns nullptr, if something went
     *                 wrong and no tool was read. In this case you may also want the error message which is availiable
     *                 from the method GetErrorMessage().
     */
    mitk::NavigationTool::Pointer DoRead(std::string filename);

    itkGetMacro(ErrorMessage,std::string);

  protected:
    NavigationToolReader();
    ~NavigationToolReader() override;

    std::string m_ErrorMessage;

    std::string m_ToolfilePath; //This path is used to store the toolfiles. It must be available through the whole MITK run.

    mitk::NavigationTool::Pointer ConvertDataNodeToNavigationTool(mitk::DataNode::Pointer node, std::string toolPath);

    //################### protected help methods ########################
    std::string GetFileWithoutPath(std::string FileWithPath);
    mitk::PointSet::Pointer ConvertStringToPointSet(std::string string);
    mitk::Point3D ConvertStringToPoint(std::string string);
    mitk::Quaternion ConvertStringToQuaternion(std::string string);
    void split(std::string& text, std::string& separators, std::vector<std::string>& words);

  };
} // namespace mitk
#endif //NAVIGATIONTOOLREADER
