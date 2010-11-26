/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKNAVIGATIONDATATOVTKCAMERAUPDATE_H_INCLUDED
#define MITKNAVIGATIONDATATOVTKCAMERAUPDATE_H_INCLUDED

#include <mitkCommon.h>
#include <mitkBaseRenderer.h>
#include <MitkIGTExports.h>
#include <mitkNavigationDataToNavigationDataFilter.h>

namespace mitk 
{
  /**Documentation
  * \brief Moves a virtual camera according to the first Navigation Data input
  *
  * This class expects the input data to be in OpenGL coordinates.
  * 
  */  
  class MitkIGT_EXPORT NavigationDataToVTKCameraUpdate: public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataToVTKCameraUpdate, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);      


    /**
    * \brief sets renderer that visualizes the navigation data
    */
    virtual void SetRenderer( mitk::BaseRenderer* renderer );
    
    /**
    * \brief returns the renderer that visualizes the navigation data
    */
    virtual const mitk::BaseRenderer* GetRenderer();


  protected:
    NavigationDataToVTKCameraUpdate();
    virtual ~NavigationDataToVTKCameraUpdate();

    /**Documentation
    * \brief filter execute method
    *
    * positions and orients camera according to the position and orientation hold in the NavigationData
    */
    virtual void GenerateData();
   
    ///
    /// renderer that visualizes the navigation data
    ///
    mitk::BaseRenderer::Pointer m_Renderer;
    ///
    /// Stores the corresponding vtk renderer
    ///
    vtkRenderer* m_vtkRenderer;
    ///
    /// Stores the active camera to be adjusted
    ///
    vtkCamera* m_Camera;
  };
} // namespace mitk
#endif
