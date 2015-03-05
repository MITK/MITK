/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef __mitkTractAnalyzer_h_
#define __mitkTractAnalyzer_h_

#include "MitkQuantificationExports.h"
#include <itkImage.h>
#include "mitkImage.h"
#include "mitkImageCast.h"

#include <mitkTbssRoiImage.h>
#include <mitkPointSet.h>


namespace mitk{
/**
  * \brief Creates a region of interest for tract-specific analysis of existing TBSS data
  *
  * This class needs a 3D image, which is the mean FA skeleton as produced by the standard TBSS pipeline of FSL.
  * How this dataset can be obtained can be found in the TBSS user manual: http://fsl.fmrib.ox.ac.uk/fsl/fsl4.0/tbss/index
  * Furthermore, this class requires a user-defined point set defining the points through which the region of interest should pass.
  * The output is a TBSS roi image, which is a binary images defining the roi and metadata containing indices
  * that can be used for plotting graphs using the QmitkTbssRoiAnalysisWidget
  */


class MITKQUANTIFICATION_EXPORT TractAnalyzer
{

public:


  TractAnalyzer();
  ~TractAnalyzer() {}


  /** Image type definitions */
  typedef itk::Image<unsigned char,3> CharImageType;
  typedef itk::Image<float,3> FloatImageType;


  /** \brief Main method for region of interest calculation
   *
   * A region of interest is calculated adding the segments between the points on the ROI
   * that was specified by the user.
   */
  void MakeRoi();



  /** \brief Returns the TbssRoiImage **/
  mitk::TbssRoiImage::Pointer GetRoiImage()
  {
    return m_TbssRoi;
  }


  /** \brief Sets the input image
   *
   * The region of interest is calculated on a 3D image. This is generally the mean FA skeleton as calculated
   * in the standard TBSS pipeline (see http://fsl.fmrib.ox.ac.uk/fsl/fslwiki/TBSS).
   */
  void SetInputImage(mitk::Image::Pointer inputImage)
  {
    m_InputImage = inputImage;
  }


  /** \brief Sets the user-defined point set
   *
   * Set the user-defined point sets. The region of interest must pass through these points.
   */
  void SetPointSet(mitk::PointSet::Pointer pointSet)
  {
    m_PointSetNode = pointSet;
  }


  /** \brief Sets a lower bound for the threshold.
   *
   * Low fractional anisotropy values can indicate partial volume of non white matter tissue.
   * This thresholds limits the search for a region of interest to voxels with a minimum value.
   */
  void SetThreshold(double threshold)
  {
    m_Threshold = threshold;
  }



  /** \brief Returns a string with the indices of points on the region of interest
   *
   * The region of interest calculated by the TractAnalyzer contains a list of ITK indices.
   * This method returns a string containing these indices for display in the GUI
   */
  std::string GetPathDescription()
  {
    return m_PathDescription;
  }

  itkGetMacro(CostSum, double)

protected:



  /** \brief Calculates a segment of the region of interest
   *
   * The region of interest is calculated on a 3D image. This is generally the mean FA skeleton as calculated
   * in the standard TBSS pipeline (see http://fsl.fmrib.ox.ac.uk/fsl/fslwiki/TBSS).
   */
  std::vector< itk::Index<3> > CreateSegment(itk::Index<3> startPoint, itk::Index<3> endPoint);


  /** \brief Output TbssRoiImage */
  mitk::TbssRoiImage::Pointer m_TbssRoi;

  /** \brief Inputimage */
  mitk::Image::Pointer m_InputImage;

  /** \brief Threshold for ROI search */
  double m_Threshold;

  /** \brief User defined point set */
  mitk::PointSet::Pointer m_PointSetNode;

  /** \brief Path description in as string for display in GUI */
  std::string m_PathDescription;

  /** \brief Total cost of the path */
  double m_CostSum;


private:

};

}

#endif //__itkTractAnalyzer_h_
