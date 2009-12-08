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

#ifndef NAVIGATIONTOOL_H_INCLUDED
#define NAVIGATIONTOOL_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>
#include <itkSpatialObject.h>

//mitk headers
#include <mitkCommon.h>
#include <mitkDataTreeNode.h>
#include <mitkTrackingTool.h>
#include <MitkIGTExports.h>

namespace mitk {
  /**Documentation
  * \brief An object of this class represents a navigation tool in the view of the software.
  *        A few informations like an identifier, a toolname, a surface and a itk spatial
  *        object are stored in such an object. The classes NavigationToolReader and
  *        are availiable to write/read tools to/from the harddisc. If you need a collection
  *        of navigation tools the class NavigationToolStorage could be used.
  *
  * \ingroup IGT
  */  
  class MitkIGT_EXPORT NavigationTool : public itk::Object
  {
  public:

    mitkClassMacro(NavigationTool,itk::Object);
    itkNewMacro(Self);

    enum NavigationToolType {Instrument, Fiducial, Skinmarker, Unkown};
    
    //## getter and setter ##
    //NavigationToolType:
    itkGetMacro(Type,NavigationToolType);
    itkSetMacro(Type,NavigationToolType);
    //Identifier:
    itkGetMacro(Identifier,std::string);
    itkSetMacro(Identifier,std::string);
    //Datatreenode:
    itkGetMacro(DataTreeNode,mitk::DataTreeNode::Pointer);
    itkSetMacro(DataTreeNode,mitk::DataTreeNode::Pointer);
    //SpatialObject:
    itkGetMacro(SpatialObject,itk::SpatialObject<3>::Pointer);
    itkSetMacro(SpatialObject,itk::SpatialObject<3>::Pointer);
    //TrackingTool:
    itkGetMacro(TrackingTool,mitk::TrackingTool::Pointer);
    itkSetMacro(TrackingTool,mitk::TrackingTool::Pointer);
    //#######################

  protected:

    NavigationTool();
    ~NavigationTool();
    
    //## 	data structure of a navigation tool object ##
    std::string m_Identifier;
    NavigationToolType m_Type;
    /** @brief This DataTreeNode holds a toolname and a tool surface */ 
    mitk::DataTreeNode::Pointer m_DataTreeNode;
    /** @brief This member variable holds a mathamatical description of the tool */
    itk::SpatialObject<3>::Pointer m_SpatialObject;
    /** @brief This member variable holds a pointer to the corresponting tracking tool in the hardware. */
    mitk::TrackingTool::Pointer m_TrackingTool;
    //#################################################

  };
} // namespace mitk
#endif //NAVIGATIONTOOL