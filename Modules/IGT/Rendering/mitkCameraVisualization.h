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

#ifndef CAMERAVISUALIZATION_H_INCLUDED
#define CAMERAVISUALIZATION_H_INCLUDED

#include "mitkCommon.h"
#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkBaseRenderer.h"

namespace mitk {
  /**Documentation
  * \brief CameraVisualization controls the camera according to the spatial information of the navigation data
  *
  * The camera of the renderer will be placed at the position of the navigation data and oriented according to
  * its orientation and the camera specific information "direction of projection", "view up", "focal length", "view angle"
  *
  * \ingroup IGT
  */  class MITKIGT_EXPORT CameraVisualization : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(CameraVisualization, NavigationDataToNavigationDataFilter);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * \brief sets the direction of projection of the camera of the renderer in tool coordinates
    */
    itkSetMacro(DirectionOfProjectionInToolCoordinates, mitk::Vector3D);

    /**
    * \brief sets the view up vector of the camera of the renderer in tool coordinates
    */
    itkSetMacro(ViewUpInToolCoordinates, mitk::Vector3D);

    /**
    * \brief sets the focal length of the camera
    */
    itkSetMacro(FocalLength, float);

    /**
    * \brief returns the direction of projection of the camera of the renderer in tool coordinates
    */
    itkGetConstMacro(DirectionOfProjectionInToolCoordinates, mitk::Vector3D);

    /**
    * \brief returns the view up vector of the camera of the renderer in tool coordinates
    */
    itkGetConstMacro(ViewUpInToolCoordinates, mitk::Vector3D);

    /**
    * \brief returns the focal length of the camera
    */
    itkGetConstMacro(FocalLength, float);

    /**
    * \brief sets renderer that visualizes the navigation data
    */
    virtual void SetRenderer( mitk::BaseRenderer* renderer );

    /**
    * \brief returns the renderer that visualizes the navigation data
    */
    virtual const mitk::BaseRenderer* GetRenderer();

    /**Documentation
    *\brief Set all filter parameters as the PropertyList p
    *
    * This method reads the following properties from the PropertyList (name : data type):
    *  - "CameraVisualization_DirectionOfProjectionInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_ViewUpInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_FocalLength" : mitk::FloatProperty
    *  - "CameraVisualization_ViewAngle" : mitk::FloatProperty
    */
    virtual void SetParameters(const mitk::PropertyList* p);

    /**Documentation
    *\brief Get all filter parameters as a PropertyList
    *
    * This method returns a PropertyList containing the following
    * properties (name : data type):
    *  - "CameraVisualization_DirectionOfProjectionInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_ViewUpInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_FocalLength" : mitk::FloatProperty
    *  - "CameraVisualization_ViewAngle" : mitk::FloatProperty
    * The returned PropertyList must be assigned to a
    * SmartPointer immediately, or else it will get destroyed.
    */
    mitk::PropertyList::ConstPointer GetParameters() const;

  protected:
    CameraVisualization();
    virtual ~CameraVisualization();

    /**Documentation
    * \brief filter execute method
    *
    * positions and orients camera according to the position and orientation hold in the NavigationData
    */
    virtual void GenerateData();

    mitk::BaseRenderer::Pointer m_Renderer; ///< renderer that visualizes the navigation data-

    Vector3D m_DirectionOfProjectionInToolCoordinates; ///< vector of the direction of projection in tool coordinates
    Vector3D m_ViewUpInToolCoordinates; ///< view up vector in tool coordinates
    Vector3D m_DirectionOfProjection; ///< vector of the current direction of view of the camera in world coordinates
    Point3D m_CameraPosition; ///< current position of the camera
    ScalarType m_FocalLength; ///< focal length of the camera: distance between camera position and focal point.
  };
} // namespace mitk
#endif
