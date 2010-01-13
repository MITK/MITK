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
  * If UseICPInitialization is false (standard value, or set with SetUseICPInitialization(false) or UseICPInitializationOff())
  * then source landmarks and target landmarks with the same ID must correspondent to each other.
  * (--> source landmark with ID x will be mapped to target landmark with ID x).
  * If you do not know the correspondences, call SetUseICPInitialization(true) or UseICPInitializationOn()
  * to let the filter guess the correspondences during initialization with an iterative closest point search.
  * This is only possible, if at least 6 source and target landmarks are available.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataLandmarkTransformFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataLandmarkTransformFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    typedef std::vector<mitk::ScalarType> ErrorVector;
    typedef itk::VersorRigid3DTransform< double > LandmarkTransformType;

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

    /** 
    *\brief Returns a vector with the euclidean distance of each transformed source point to its respective target point
    *
    */
    const ErrorVector& GetErrorVector() const; 

    itkSetMacro(UseICPInitialization, bool); ///< If set to true, source and target point correspondences are established with iterative closest point optimization
    itkGetMacro(UseICPInitialization, bool); ///< If set to true, source and target point correspondences are established with iterative closest point optimization
    itkBooleanMacro(UseICPInitialization);   ///< If set to true, source and target point correspondences are established with iterative closest point optimization

    itkGetConstObjectMacro(LandmarkTransform, LandmarkTransformType);  ///< returns the current landmark transform

  protected:
    typedef itk::Image< signed short, 3>  ImageType;       // only because itk::LandmarkBasedTransformInitializer must be templated over two imagetypes
    
    typedef itk::LandmarkBasedTransformInitializer< LandmarkTransformType, ImageType, ImageType > TransformInitializerType;
    typedef TransformInitializerType::LandmarkPointContainer LandmarkPointContainer; 
    typedef itk::QuaternionRigidTransform<double> QuaternionTransformType;

    /**
    * \brief Constructor 
    **/
    NavigationDataLandmarkTransformFilter();
    virtual ~NavigationDataLandmarkTransformFilter();

    /**
    * \brief transforms input NDs according to the calculated LandmarkTransform 
    * 
    */
    virtual void GenerateData();

    /**Documentation
    * \brief perform an iterative closest point matching to find corresponding landmarks that will be used for landmark transform calculation
    *
    * Perform ICP optimization to match source landmarks to target landmarks. Landmark containers must contain
    * at least 6 landmarks for the optimization.
    * after ICP, landmark correspondences are established and the source landmarks are sorted, so that 
    * corresponding landmarks have the same indices.
    * 
    * \param[in] sources Source landmarks that will be mapped to the target landmarks
    * \param[in] targets Target landmarks onto which the source landmarks will be mapped
    * \param[out] sources The sources container will be sorted, 
                  so that landmarks have the same index as their corresponding target landmarks
    * \return true if ICP was successful and sources are sorted , false otherwise
    */
    bool FindCorrespondentLandmarks(LandmarkPointContainer& sources, const LandmarkPointContainer& targets) const; 

    /**
    * \brief initializes the transform using source and target PointSets
    *
    * if UseICPInitialization is true, FindCorrespondentLandmarks() will be used to sort the source landmarks in order to 
    * establish corresponding landmark pairs before the landmark transform is build
    */
    void InitializeLandmarkTransform(LandmarkPointContainer& sources, const LandmarkPointContainer& targets);
    
    /**
    * \brief calculates the transform using source and target PointSets
    */
    void UpdateLandmarkTransform(const LandmarkPointContainer &sources, const  LandmarkPointContainer &targets); ///<
    void AccumulateStatistics(ErrorVector& vector); ///< calculate error metrics for the transforms.

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
    LandmarkTransformType::Pointer m_LandmarkTransform;               ///<  transform calculated from source and target points

    QuaternionTransformType::Pointer m_QuatLandmarkTransform; ///< transform needed to rotate orientation
    QuaternionTransformType::Pointer m_QuatTransform;         ///< further transform needed to rotate orientation

    ErrorVector m_Errors; ///< stores the euclidean distance of each transformed source landmark and its respective target landmark
    bool m_UseICPInitialization; ///< find source <--> target point correspondences with iterative closest point optimization
  };
} // namespace mitk
#endif /* MITKNavigationDataLandmarkTransformFilter_H_HEADER_INCLUDED_ */
