/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKNavigationDataLandmarkTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataLandmarkTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include <mitkPointSet.h>
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkQuaternionRigidTransform.h>
#include <itkImage.h>


namespace mitk {

  /**Documentation
  * \brief NavigationDataLandmarkTransformFilter applies a itk-landmark-transformation
  * defined by source and target pointsets. 
  * 
  * Before executing the filter SetSourcePoints and SetTargetPoints must be called. Before both source 
  * and target landmarks are set, the filter performs an identity transform.
  * If source or target points are changed after calling SetXXXPoints, the corresponding SetXXXPoints 
  * method has to be called again to apply the changes.
  *
  * \ingroup IGT
  */
  class MITK_IGT_EXPORT NavigationDataLandmarkTransformFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataLandmarkTransformFilter, NavigationDataToNavigationDataFilter);

    /**
    * \brief Constructor 
    **/
    NavigationDataLandmarkTransformFilter();
    virtual ~NavigationDataLandmarkTransformFilter();

    itkNewMacro(Self);
  
    /**Documentation 
    *\brief Set points used as source points for landmark transform.
    *
    */
    void SetSourcePoints(mitk::PointSet::Pointer sourcePointSet);
    
    /**Documentation 
    *\brief Set points used as target points for landmark transform
    *
    */
    void SetTargetPoints(mitk::PointSet::Pointer targetPointSet);

  protected:
    typedef itk::Image< signed short, 3>  ImageType;       // only because itk::LandmarkBasedTransformInitializer must be templated over two imagetypes
    typedef itk::VersorRigid3DTransform< double > ITKVersorTransformType;
    typedef itk::LandmarkBasedTransformInitializer< ITKVersorTransformType, ImageType, ImageType > TransformInitializerType;

    TransformInitializerType::LandmarkPointContainer m_SourcePoints;      ///<  positions of the source points
    TransformInitializerType::LandmarkPointContainer m_TargetPoints;      ///<  positions of the target points
    TransformInitializerType::Pointer m_LandmarkTransformInitializer;     ///<  landmark based transform initializer 
    ITKVersorTransformType::Pointer m_ITKLandmarkTransform;               ///<  transform calculated from source and target points

    itk::QuaternionRigidTransform<double>::Pointer m_QuatLandmarkTransform; ///< transform needed to rotate orientation
    itk::QuaternionRigidTransform<double>::Pointer m_QuatTransform;         ///< further transform needed to rotate orientation

    /**
    * \brief transforms input NDs according to the calculated landmarktransform 
    * 
    */
    virtual void GenerateData();

    /**
    * \brief initializes the transform using source and target pointsets 
    */
    void InitializeLandmarkTransform();

    bool m_SourcePointsAreSet; ///< bool to store if source points were set
    bool m_TargetPointsAreSet; ///< bool to store if target points were set 
  };
} // namespace mitk
#endif /* MITKNavigationDataLandmarkTransformFilter_H_HEADER_INCLUDED_ */
