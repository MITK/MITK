/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKNavigationDataReferenceTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataReferenceTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataLandmarkTransformFilter.h>

#include <itkQuaternionRigidTransform.h>
#include <mitkPointSet.h>



namespace mitk {

  /**Documentation
  * \brief NavigationDataReferenceTransformFilter applies a itk-landmark-transformation
  * defined by source and target NavigationDatas. 
  * 
  * Before executing the filter SetSourceNavigationDatas and SetTargetNavigationDatas must be called. 
  * The amount of given NavigationDatas must be the same for source and target.
  * If source or target points are changed after calling SetXXXNavigationDatas, the corresponding SetXXXNavigationDatas 
  * method has to be called again to apply the changes.
  * If UseICPInitialization is false (standard value, or set with SetUseICPInitialization(false) or UseICPInitializationOff())
  * then source landmarks and target landmarks with the same ID must correspondent to each other.
  * (--> source landmark with ID x will be mapped to target landmark with ID x).
  * If you do not know the correspondences, call SetUseICPInitialization(true) or UseICPInitializationOn()
  * to let the filter guess the correspondences during initialization with an iterative closest point search.
  * This is only possible, if at least 6 source and target landmarks are available.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataReferenceTransformFilter : public NavigationDataLandmarkTransformFilter
  {
  public:
    mitkClassMacro(NavigationDataReferenceTransformFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    typedef itk::QuaternionRigidTransform<double> QuaternionTransformType; ///< Quaternion transform for calculating 

    /** 
    *\brief Set NavigationDatas whose positions are used as source points for the transform.
    *
    */
    void SetSourceNavigationDatas(const std::vector<mitk::NavigationData::Pointer>& sourceNavigationDatas);

    /** 
    *\brief Set NavigationDatas whose positions are used as target points for the transform.
    *
    */
    void SetTargetNavigationDatas(const std::vector<mitk::NavigationData::Pointer>& sourceNavigationDatas);

    /** 
    *\brief Sets the filter back to initial settings.
    *
    */
    void ReinitFilter();

    /** 
    *\brief Returns the source landmarks PointSet filled with points from given ND position(s) and orientation
    */
    const mitk::PointSet::Pointer GetSourceLandmarks(); 
    /** 
    *\brief Returns the target landmarks PointSet filled with points from given ND position(s) and orientation
    */
    const mitk::PointSet::Pointer GetTargetLandmarks();

    /**
    * \brief Initializes the transform. Transform will be perfomed only if source and target points have the same size.
    *
    * Returns "true" if transform has been initialized, else "false" is returned.
    **/
    bool InitializeTransform();


  protected:

    /**
    * \brief Constructor
    **/
    NavigationDataReferenceTransformFilter();
    /**
    * \brief Destructor
    **/
    virtual ~NavigationDataReferenceTransformFilter();

    QuaternionTransformType::Pointer m_QuaternionTransform; ///< itk Quaternion transform

    mitk::PointSet::Pointer CreateLandmarkPointsForSingleNavigationData(mitk::PointSet::Pointer landmarkContainer, const std::vector<mitk::NavigationData::Pointer>& navigationDatas);

    mitk::PointSet::Pointer m_SourceLandmarksFromNavigationDatas; ///< source points from NavigationDatas
    mitk::PointSet::Pointer m_TargetLandmarksFromNavigationDatas; ///< target points from NavigationDatas


  };
} // namespace mitk
#endif /* MITKNavigationDataReferenceTransformFilter_H_HEADER_INCLUDED_ */
