/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef CAMERAVISUALIZATION_H_INCLUDED
#define CAMERAVISUALIZATION_H_INCLUDED

#include "mitkCommon.h"
#include "mitkNavigationDataVisualizationFilter.h"
#include "mitkVtkPropRenderer.h"

namespace mitk {
  /**Documentation
  * \brief CameraVisualization controls the camera according to the spatial information of the navigation data
  *
  * The camera of the renderer will be placed at the position of the navigation data and oriented according to
  * its orientation and the camera specific information "direction of projection", "view up", "focal length", "view angle"
  *
  * @ingroup Navigation
  */  class CameraVisualization : public NavigationDataVisualizationFilter
  {
  public:
    mitkClassMacro(CameraVisualization,NavigationDataVisualizationFilter);
    itkNewMacro(Self);

    /**Documentation
    * \brief sets the direction of projection of the camera of the renderer in tool coordinates
    */
    itkSetMacro(DirectionOfProjectionInToolCoordinates,Vector3D);
    /**Documentation
    * \brief sets the view up vector of the camera of the renderer in tool coordinates
    */
    itkSetMacro(ViewUpInToolCoordinates,Vector3D);
    /**Documentation
    * \brief sets the focal length of the camera
    */
    itkSetMacro(FocalLength,float);
    /**Documentation
    * \brief sets the view angle of the camera of the renderer which angular height of the camera view measured in degrees (cf. vtkCamera)
    */
    itkSetMacro(ViewAngle,float);
    /**Documentation
    * \brief returns the direction of projection of the camera of the renderer in tool coordinates
    */
    itkGetConstMacro(DirectionOfProjectionInToolCoordinates,Vector3D);
    /**Documentation
    * \brief returns the view up vector of the camera of the renderer in tool coordinates
    */
    itkGetConstMacro(ViewUpInToolCoordinates,Vector3D);
    /**Documentation
    * \brief returns the focal length of the camera
    */
    itkGetConstMacro(FocalLength,float);
    /**Documentation
    * \brief returns the view angle of the camera of the renderer which angular height of the camera view measured in degrees (cf. vtkCamera)
    */
    itkGetConstMacro(ViewAngle,float);
    /**Documentation
    * \brief sets renderer that visualizes the navigation data
    */
    virtual void SetRenderer( VtkPropRenderer::Pointer renderer );
    
    /**Documentation
    * \brief returns the renderer that visualizes the navigation data
    */
    virtual const BaseRenderer* GetRenderer();

    /**Documentation 
    *@brief Set all filter parameters as the PropertyList p
    * This method reads the following properties from the PropertyList (name : data type):
    *  - "CameraVisualization_DirectionOfProjectionInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_ViewUpInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_FocalLength" : mitk::FloatProperty
    *  - "CameraVisualization_ViewAngle" : mitk::FloatProperty
    */
    virtual void SetParameters(const mitk::PropertyList* p);

    /**Documentation 
    *@brief Get all filter parameters as a PropertyList
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
    /**Documentation
    * \brief filter execute method
    * positions and orients camera according to the position and orientation hold in the NavigationData
    */
    virtual void GenerateData();

  protected:
    CameraVisualization();
    virtual ~CameraVisualization();

    BaseRenderer* m_Renderer; ///< renderer that visualizes the navigation data

    Vector3D m_DirectionOfProjectionInToolCoordinates; ///< vector of the direction of projection in tool coordinates
    Vector3D m_ViewUpInToolCoordinates; ///< view up vector in tool coordinates
    float m_FocalLength; ///< focal length of the camera: distance between camera position and focal point.
    float m_ViewAngle; ///< view angle of the camera: angular height of the camera view measured in degrees.
  };
} // namespace mitk


#endif
