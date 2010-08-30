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

#ifndef MITKNavigationDataReferenceTransformFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataReferenceTransformFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include <mitkPointSet.h>
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkImage.h>
#include <itkFixedCenterOfRotationAffineTransform.h>

namespace mitk {

  /**Documentation
  * \brief NavigationDataReferenceTransformFilter applies a itk-landmark-transformation defined by 3 (or more) source and target points. 
  *      If there are less than 3 source/target points available the missing landmark points can be calculated by using the ND of only one input.
  * 
  * Before executing the filter the ReferenceSourceIndexes vector with the reference input IDs must be set and GenerateReferenceSourceLandmarks and GenerateReferenceTargetLandmarks have to be called.
  * The filter can also be executed if SetSourceLandmarks and SetTargetLandmarks are called and there are at least 3 source and 3 target landmark points set.
  * If source or target points are changed after calling GenerateReferenceXXXLandmarks or SetXXXPoints the corresponding method has to be called again to apply the changes. 
  * By using the landmark point generator methods moving points are updated on every Update() of this filter.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataReferenceTransformFilter : public NavigationDataToNavigationDataFilter
  {
  public:

    typedef itk::VersorRigid3DTransform< double > LandmarkTransformType; ///< 
    typedef std::vector<int> ReferenceInputsType;
    typedef itk::Image< signed short, 3>  ImageType;       // only because itk::LandmarkBasedTransformInitializer must be templated over two imagetypes        
    typedef itk::QuaternionRigidTransform<double> QuaternionTransformType;
    typedef itk::LandmarkBasedTransformInitializer< LandmarkTransformType, ImageType, ImageType > TransformInitializerType; 
    typedef TransformInitializerType::LandmarkPointContainer LandmarkPointContainer; 

    mitkClassMacro(NavigationDataReferenceTransformFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    itkGetMacro(OneSourceRegistration, bool);
    itkGetMacro(ReferenceRegistration, bool);

    const LandmarkPointContainer GetSourcePoints();
    const LandmarkPointContainer GetTargetPoints();
    const LandmarkPointContainer GetReferencePoints();
    /**
    *\brief Sets the vector with the reference input indexes. 
    * 
    * If there are less than 3 reference inputs indexes m_OneSourceRegistration is set true and InitializeReferenceLandmarks() is called. 
    */
    void SetReferenceInputIndexes(const std::vector<int>* indexes);
    
    /**
    *\brief Returns a vector with the reference inputs IDs.
    *
    */
    const std::vector<int> GetReferenceInputIndexes();  
   
    /** 
    *\brief Set points used as source points for landmark transform.
    *
    */
    virtual void SetSourceLandmarks(mitk::PointSet::Pointer sourcePointSet);

    /** 
    *\brief Set points used as source points for landmark transform.
    *
    */
    virtual void SetTargetLandmarks(mitk::PointSet::Pointer targetPointSet);

    /** 
    *\brief Assigns at least 3 landmarks to the source landmarks container. 
    *
    * Therefor the positions of the reference inputs are used. If there are less than 3 reference inputs 2 landmarks will be calculated in dependency of the first reference input.
    */
    void GenerateSourceLandmarks();
    
    /* 
    *\brief Assigns at least 3 landmarks to the target landmarks container.
    *
    * Therefor the positions of the reference inputs are used. If there are less than 3 reference inputs 2 landmarks will be calculated in dependency of the first reference input.
    */
    void GenerateTargetLandmarks();

    /**
    *\brief Checks if there are enough source and target points for a transform and if source and target point amount are equal.
    *
    */
    virtual bool IsInitialized() const;

    /**
    *\brief Fills the reference landmark container which is needed for the quaternion transform. 
    *
    * This method is called if there are less than 3 reference inputs.
    */
    bool InitializeReferenceLandmarks();

    /**
    *\brief Resets the filter to it's initial state.
    *
    * Clears all landmarks and resets the outputs by calling CreateOutputsForAllInputs() from superclass
    */
    void ResetFilter();


  protected:

    /**
    * \brief Constructor 
    **/
    NavigationDataReferenceTransformFilter();

    /**
    * \brief Destructor 
    **/
    virtual ~NavigationDataReferenceTransformFilter();

    /**
    * \brief Calculates the transform using source and target PointSets.
    *
    * If there are at least 3 corresponding source and target points a transform will be performed.
    */
    void UpdateLandmarkTransform(const LandmarkPointContainer &sources, const  LandmarkPointContainer &targets);
    
    /**
    * \brief Transforms input NDs according to the calculated LandmarkTransform.
    */
    virtual void GenerateData();
    
    void PrintSelf( std::ostream& os, itk::Indent indent ) const;  // print object info to ostream

    /**
    * \brief  Generates landmarks from the actual reference inputs ND.
    *
    * Generates as many landmarks as there are reference inputs. If there are less than 3 reference inputs 2 virtual landmarks are calculated by using the ND of the first reference input.
    */
    const LandmarkPointContainer GenerateReferenceLandmarks();
    

    LandmarkPointContainer m_SourcePoints;    /// positions of the source points
    LandmarkPointContainer m_TargetPoints;    /// positions of the target points
    LandmarkPointContainer m_ReferencePoints; /// positions of the reference points

    TransformInitializerType::Pointer m_LandmarkTransformInitializer; /// landmark based transform initializer 
    LandmarkTransformType::Pointer m_LandmarkTransform; /// transform calculated from source and target points

    QuaternionTransformType::Pointer m_QuaternionTransform; /// further transform needed to rotate orientation
    QuaternionTransformType::Pointer m_QuaternionLandmarkTransform;  /// transform needed to rotate orientation

    QuaternionTransformType::Pointer m_QuaternionReferenceTransform;  /// transform needed to rotate the virtual reference points

    bool m_OneSourceRegistration;  /// bool needed to show that there are less than 3 reference inputs
    bool m_ReferenceRegistration; /// bool needed to show if landmarks are generated (true) or set (false)
    
    ReferenceInputsType m_ReferenceInputIndexes; /// the input ids of the reference inputs

  };
} // namespace mitk
#endif /* MITKNavigationDataReferenceTransformFilter_H_HEADER_INCLUDED_ */

