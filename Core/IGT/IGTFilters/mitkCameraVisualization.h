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
  class CameraVisualization : public NavigationDataVisualizationFilter
  {
  public:
    mitkClassMacro(CameraVisualization,NavigationDataVisualizationFilter);
    itkNewMacro(Self);

    itkSetMacro(DirectionOfProjectionInToolCoordinates,Vector3D);
    itkSetMacro(ViewUpInToolCoordinates,Vector3D);
    itkSetMacro(FocalLength,float);
    itkGetConstMacro(DirectionOfProjectionInToolCoordinates,Vector3D);
    itkGetConstMacro(ViewUpInToolCoordinates,Vector3D);
    itkGetConstMacro(FocalLength,float);
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
    */
    virtual void SetParameters(const mitk::PropertyList* p);

    /**Documentation 
    *@brief Get all filter parameters as a PropertyList
    *
    * This method returns a PropertyList containing the following
    * properties (name : data type):
    *  - "CameraVisualization_DirectionOfProjectionInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_ViewUpInToolCoordinates" : mitk::Vector3DProperty
    *  - "CameraVisualization_FocalLength" : mitk::FloatProperty
    * The returned PropertyList must be assigned to a 
    * SmartPointer immediately, or else it will get destroyed.
    */
    mitk::PropertyList::ConstPointer GetParameters() const;
    /**Documentation
    * \brief filter execute method
    *
    * positions and orients camera according to NavigationData
    */
    virtual void Update();

  protected:
    CameraVisualization();
    virtual ~CameraVisualization();

    ///< renderer that visualizes the navigation data
    const BaseRenderer* m_Renderer; 

    Vector3D m_DirectionOfProjectionInToolCoordinates; ///< vector of the direction of projection in tool coordinates
    Vector3D m_ViewUpInToolCoordinates; ///< view up vector in tool coordinates
    float m_FocalLength; ///< focal length of the camera = distance from camera position to focal point in mm.
  };
} // namespace mitk


#endif
