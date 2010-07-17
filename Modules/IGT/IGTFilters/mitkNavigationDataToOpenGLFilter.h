/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKNavigationDataToOpenGLFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataToOpenGLFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include <mitkPointSet.h>
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkQuaternionRigidTransform.h>
#include <itkImage.h>


namespace mitk {

  /**Documentation
  * \brief A filter that transforms input NavigationData that are defined in a left-handed
  * coordinate system into a right-handed coordinate system like OpenGL
  *
  * OpenGL uses a right handed coordinate system (\see http://www2.evl.uic.edu/ralph/508S98/coordinates.html).
  * When using IGT for Augmented Reality applications the transform of navigation datas
  * into the OpenGL coordinate system is mandatory.
  * \see mitk::CameraVisualizationFilter
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataToOpenGLFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataToOpenGLFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);
    /**
    * \brief Constructor 
    **/
    NavigationDataToOpenGLFilter();
    virtual ~NavigationDataToOpenGLFilter();

    /**
    * \brief transforms input NDs according to the calculated LandmarkTransform 
    * 
    */
    virtual void GenerateData();

  };
} // namespace mitk
#endif /* MITKNavigationDataToOpenGLFilter_H_HEADER_INCLUDED_ */
