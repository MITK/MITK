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

#ifndef NAVIGATIONTOOL_H_INCLUDED
#define NAVIGATIONTOOL_H_INCLUDED

//itk headers
#include <itkObjectFactory.h>
#include <itkSpatialObject.h>
#include <itkDataObject.h>

//mitk headers
#include <mitkCommon.h>
#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkTrackingTool.h>
#include <mitkTrackingTypes.h>
#include <mitkSurface.h>
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
  class MitkIGT_EXPORT NavigationTool : public itk::DataObject
  {
  public:

    mitkClassMacro(NavigationTool,itk::DataObject);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    enum NavigationToolType {Instrument, Fiducial, Skinmarker, Unknown};

    //## getter and setter ##
    //NavigationToolType:
    itkGetConstMacro(Type,NavigationToolType);
    itkSetMacro(Type,NavigationToolType);

    //Identifier:
    itkGetConstMacro(Identifier,std::string);
    itkSetMacro(Identifier,std::string);

    //Datatreenode:
    itkGetConstMacro(DataNode,mitk::DataNode::Pointer);
    itkSetMacro(DataNode,mitk::DataNode::Pointer);

    //SpatialObject:
    itkGetConstMacro(SpatialObject,itk::SpatialObject<3>::Pointer);
    itkSetMacro(SpatialObject,itk::SpatialObject<3>::Pointer);

    //TrackingTool:
    itkGetConstMacro(TrackingTool,mitk::TrackingTool::Pointer);
    itkSetMacro(TrackingTool,mitk::TrackingTool::Pointer);

    //CalibrationFile:
    itkGetConstMacro(CalibrationFile,std::string);
    void SetCalibrationFile(const std::string filename);

    //Tool tip definition:
    itkGetConstMacro(ToolTipPosition,mitk::Point3D);
    itkSetMacro(ToolTipPosition,mitk::Point3D);
    itkGetConstMacro(ToolTipOrientation,mitk::Quaternion);
    itkSetMacro(ToolTipOrientation,mitk::Quaternion);

    /** @return Returns the tooltip as transform object. */
    mitk::AffineTransform3D::Pointer GetToolTipTransform();

    /** @return Returns true if a tooltip is set, false if not. */
    bool IsToolTipSet();

    //Tool Landmarks:
    /** For overview, here are descriptons of the two types of tool landmarks:
     *
     *  tool calibration landmarks: These landmarks may be used clearly define the tools pose only by
     *  using landmarks in the tool coordinate system. E.g., two landmarks for a 5DoF tool and three
     *  landmarks for a 6DoF tool. These landmarks may be used, e.g., for a point based registration
     *  of a tool from image space to tracking space.
     *
     *  tool registration landmarks: These landmarks are designed for representing defined landmarks
     *  on a tools surface. The number of these landmarks might exeed the number of tool calibration
     *  landmarks for reasons of redundancy and averaging. They are used for, e.g., manually registering
     *  the pose of a tool by visual markers in a CT scan. If you would use these landmarks to do a
     *  point based registration from image space to tracking space later, you might overweight the
     *  tool because of two many landmarks compared to other markers.
     *
     *  @return Returns the tool registration landmarks which represent markers / special points on a
     *          tool that can be used for registration. The landmarks should be given in tool coordinates.
     *          If there are no landmarks defined for this tool the method returns an empty point set.
     */
    itkGetConstMacro(ToolRegistrationLandmarks,mitk::PointSet::Pointer);
    /** @brief  Sets the tool registration landmarks which represent markers / special points on a
     *          tool that can be used for registration. The landmarks should be given in tool coordinates.
     */
    itkSetMacro(ToolRegistrationLandmarks,mitk::PointSet::Pointer);
    /** @return Returns the tool calibration landmarks for calibration of the defined points in the
      *         tool coordinate system, e.g. 2 landmarks for a 5DoF tool and 3 landmarks for a 6DoF tool.
      */
    itkGetConstMacro(ToolCalibrationLandmarks,mitk::PointSet::Pointer);
    /** @brief  Sets the tool calibration landmarks for calibration of defined points in the
      *         tool coordinate system, e.g. 2 landmarks for a 5DoF tool and 3 landmarks for a 6DoF tool.
      */
    itkSetMacro(ToolCalibrationLandmarks,mitk::PointSet::Pointer);

    //SerialNumber:
    itkGetConstMacro(SerialNumber,std::string);
    itkSetMacro(SerialNumber,std::string);
    //TrackingDeviceType:
    itkGetConstMacro(TrackingDeviceType,mitk::TrackingDeviceType);
    itkSetMacro(TrackingDeviceType,mitk::TrackingDeviceType);
    //ToolName (only getter):
    /** @return Returns the name of this navigation tool. Returns an empty string if there is
     *          no name (for example because the data node has not been set yet).
     *
     *          Note: There is no setter for the name,
     *          because the name of the corresponding data node is used as tool name. So if you
     *          want to modify the name of this navigation tool only get the data node and modify
     *          its name.
     */
    std::string GetToolName();
    //ToolSurface (only getter):
    /** @return Returns the surface of this navigation tool. Returns NULL if there is
     *          no surface (for example because the data node has not been set yet).
     *
     *          Note: There is no setter for the surface,
     *          because the surface is the data of the corresponding data node. So if you
     *          want to set a new surface only get the data node and modify its data.
     */
    mitk::Surface::Pointer GetToolSurface();
    /**
      * \brief Graft the data and information from one NavigationTool to another.
      *
      * Copies the content of data into this object.
      * This is a convenience method to setup a second NavigationTool object with all the meta
      * information of another NavigationTool object.
      * Note that this method is different than just using two
      * SmartPointers to the same NavigationTool object since separate DataObjects are
      * still maintained.
      */
    virtual void Graft(const DataObject *data);
    //#######################

  protected:

    NavigationTool();
    ~NavigationTool();

    //## data structure of a navigation tool object ##
    std::string m_Identifier;
    NavigationToolType m_Type;
    /** @brief This DataNode holds a toolname and a tool surface */
    mitk::DataNode::Pointer m_DataNode;
    /** @brief This member variable holds a mathamatical description of the tool */
    itk::SpatialObject<3>::Pointer m_SpatialObject;
    /** @brief This member variable holds a pointer to the corresponding tracking tool in the hardware. */
    mitk::TrackingTool::Pointer m_TrackingTool;
    /** @brief The path to the calibration file of the tool. */
    std::string m_CalibrationFile;
    /** @brief A unique serial number of the tool which is needed to identify the tool correctly. This is very important
      *        in case of the NDI Aurora System. */
    std::string m_SerialNumber;
    /** @brief   This member holds the tracking device type of the tool. */
    mitk::TrackingDeviceType m_TrackingDeviceType;
    /** @brief Holds landmarks for tool registration. */
    mitk::PointSet::Pointer m_ToolRegistrationLandmarks;
    /** @brief Holds landmarks for calibration of the defined points in the tool coordinate system,
      *        e.g. 2 landmarks for a 5DoF tool and 3 landmarks for a 6DoF tool.
      */
    mitk::PointSet::Pointer m_ToolCalibrationLandmarks;
    /** @brief Holds the position of the tool tip. */
    mitk::Point3D m_ToolTipPosition;
    /** @brief Holds the orientation of the tool tip. */
    mitk::Quaternion m_ToolTipOrientation;
    //#################################################

  };
} // namespace mitk
#endif //NAVIGATIONTOOL
