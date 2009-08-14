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
  class MitkIGT_EXPORT NavigationDataLandmarkTransformFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataLandmarkTransformFilter, NavigationDataToNavigationDataFilter);

    /**
    * \brief Constructor 
    **/
    NavigationDataLandmarkTransformFilter();
    virtual ~NavigationDataLandmarkTransformFilter();

    itkNewMacro(Self);
  
    /** 
    *\brief Set points used as source points for landmark transform.
    *
    */
    virtual void SetSourcePoints(mitk::PointSet::Pointer sourcePointSet);

    /** 
    *\brief Set points used as target points for landmark transform
    *
    */
    virtual void SetTargetPoints(mitk::PointSet::Pointer targetPointSet);

    virtual bool IsInitialized() const;

    /** 
    *\brief Returns the Fiducial Registration Error
    *
    */
    mitk::ScalarType GetFRE() const;

    /** 
    *\brief Returns the standard deviation of the Fiducial Registration Error
    *
    */
    mitk::ScalarType GetFREStdDev() const;
    
    /** 
    *\brief Returns the Root Mean Square of the registration error
    *
    */    
    mitk::ScalarType GetRMSError() const;
    
    /** 
    *\brief Returns the minimum registration error / best fitting landmark distance
    *
    */    
    mitk::ScalarType GetMinError() const;
    
    /** 
    *\brief Returns the maximum registration error / worst fitting landmark distance
    *
    */   
    mitk::ScalarType GetMaxError() const;
    
    /** 
    *\brief Returns the absolute maximum registration error
    *
    */    
    mitk::ScalarType GetAbsMaxError() const;

  protected:
    typedef itk::Image< signed short, 3>  ImageType;       // only because itk::LandmarkBasedTransformInitializer must be templated over two imagetypes
    typedef itk::VersorRigid3DTransform< double > ITKVersorTransformType;
    typedef itk::LandmarkBasedTransformInitializer< ITKVersorTransformType, ImageType, ImageType > TransformInitializerType;
    typedef TransformInitializerType::LandmarkPointContainer LandmarkPointContainer; 
    typedef itk::QuaternionRigidTransform<double> QuaternionTransformType;
    /**
    * \brief transforms input NDs according to the calculated LandmarkTransform 
    * 
    */
    virtual void GenerateData();

    /**
    * \brief initializes the transform using source and target PointSets
    */
    void InitializeLandmarkTransform(LandmarkPointContainer sources, LandmarkPointContainer targets);

    void AccumulateStatistics(std::vector<mitk::ScalarType>& vector); ///< calculate error metrics for the transforms. 
    void PrintSelf( std::ostream& os, itk::Indent indent ) const;     ///< print object info to ostream

    mitk::ScalarType m_ErrorMean;     ///< Fiducial Registration Error
    mitk::ScalarType m_ErrorStdDev;   ///< standard deviation of the Fiducial Registration Error
    mitk::ScalarType m_ErrorRMS;      ///< Root Mean Square of the registration error
    mitk::ScalarType m_ErrorMin;      ///< minimum registration error / best fitting landmark distance
    mitk::ScalarType m_ErrorMax;      ///< maximum registration error / worst fitting landmark distance
    mitk::ScalarType m_ErrorAbsMax;   ///< the absolute maximum registration error

    LandmarkPointContainer m_SourcePoints;      ///<  positions of the source points
    LandmarkPointContainer m_TargetPoints;      ///<  positions of the target points
    TransformInitializerType::Pointer m_LandmarkTransformInitializer;     ///<  landmark based transform initializer 
    ITKVersorTransformType::Pointer m_ITKLandmarkTransform;               ///<  transform calculated from source and target points

    QuaternionTransformType::Pointer m_QuatLandmarkTransform; ///< transform needed to rotate orientation
    QuaternionTransformType::Pointer m_QuatTransform;         ///< further transform needed to rotate orientation


    bool m_SourcePointsAreSet; ///< bool to store if source points were set
    bool m_TargetPointsAreSet; ///< bool to store if target points were set 
  };
} // namespace mitk
#endif /* MITKNavigationDataLandmarkTransformFilter_H_HEADER_INCLUDED_ */
