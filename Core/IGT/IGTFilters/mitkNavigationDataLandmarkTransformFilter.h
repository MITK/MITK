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
#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>
#include <vector>


namespace mitk {

  /**Documentation
  * \brief NavigationDataLandmarkTransformFilter applies a vtk-landmark-transformation defined by (three or more) reference ND on all ND objects
  *
  * @ingroup Navigation
  */
  class NavigationDataLandmarkTransformFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataLandmarkTransformFilter, NavigationDataToNavigationDataFilter);

    /**
    * @brief Constructor for initializing filter --> using NavigationDataIDs to 
    * to specify the reference tools (NavigationDatas).
    * ToDo: Default constructor uses ND parameter "TYPE"
    **/
    NavigationDataLandmarkTransformFilter(std::vector<int> IDs);

    itkNewMacro(Self);
  
    typedef vtkLandmarkTransform TransformType;


  protected:
    NavigationDataLandmarkTransformFilter();
    virtual ~NavigationDataLandmarkTransformFilter();

    /**Documentation
    * \brief filter execute method
    *
    * initializes reference values and calls TransformNavigationDatas   
    */
    virtual void GenerateData();

    /**Documentation
    * \brief check whether ALL navigationDatas are valid
    *
    * returns false if one NavigationData is invalid
    */
    bool AllNavigationDatasValid();

    /**Documentation
    * \brief insert point values of the reference ND into "points"
    */
    void FillPointsWithCurrentNDReferencePoints(vtkPoints* points);
    
    /**Documentation
    * \brief calculates the vtk-landmark-transform 
    */
    bool CalculateTransform();

    /**Documentation
    * \brief transforms all NDs 
    */
    void TransformNavigationDatas(TransformType* landmarkTransform);

    // vtk-landmark-transform 
    TransformType* m_LandmarkTransform; 

    //vtkPoints to hold the position of the initial fiducial markers inorder to calculate a landmark-transform using the reference NDs
    vtkPoints* m_FiducialMarkers;

    // bool to store if initial fiducial markers are set
    bool m_FiducialMarkersAreSet;
    
    /* 
    * array containing the NavigationDataIDs used as reference tools
    * only used if filter was initialized with "NavigationDataLandmarkTransformFilter(int *navigationDataIds)"
    */
    int *m_ReferenceNavigationDataIDs;

    std::vector<int> m_IDs;
    
    // flag for using m_ReferenceNavigationDataIDs as reference tools
    bool m_UseNavigationDataIDasReference;
  };
} // namespace mitk

#endif /* MITKNavigationDataLandmarkTransformFilter_H_HEADER_INCLUDED_ */
