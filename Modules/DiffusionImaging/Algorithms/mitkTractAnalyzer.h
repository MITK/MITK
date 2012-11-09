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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __mitkTractAnalyzer_h_
#define __mitkTractAnalyzer_h_

#include "MitkDiffusionImagingExports.h"
#include <itkImage.h>
#include <mitkTbssImage.h>


namespace mitk{
/** \class TractAnalyzer
 */


class MitkDiffusionImaging_EXPORT TractAnalyzer
{

public:


  TractAnalyzer();
  ~TractAnalyzer() {};

  typedef itk::Image<char,3> CharImageType;
  typedef itk::Image<float,3> FloatImageType;
  typedef itk::Image<float,4> ProjectionsImageType;
  typedef itk::VectorImage<float, 3> VectorImageType;



  /*
  void SetSkeleton(CharImageType::Pointer skeleton)
  {
    m_Skeleton = skeleton;
  }

  void SetMeanSkeleton(FloatImageType::Pointer i)
  {
    m_MeanSkeleton = i;
  }*/


  void SetTbssImage(mitk::TbssImage::Pointer tbssImg)
  {
    m_TbssImage = tbssImg;
  }

  void SetProjections(ProjectionsImageType::Pointer projections)
  {
    m_Projections = projections;
  }

  void BuildGraph(itk::Index<3> startPoint, itk::Index<3> endPoint);

  std::vector< itk::Index<3> > GetPath()
  {
    return m_Path;
  }

  void SetFileName(std::string fname)
  {
    m_FileName = fname;
  }

  void SetFileNameLong(std::string fname)
  {
    m_FileNameLong = fname;
  }

  void SetRoi(std::vector< itk::Index<3> > roi)
  {
    m_Roi = roi;
  }

  CharImageType::Pointer GetRoiImage()
  {
    return m_RoiImg;
  }

  void SetGroups(std::vector< std::pair<std::string, int> > groups)
  {
    m_Groups = groups;
  }

  void MeasureRoi();

  void SetInputImage(mitk::Image::Pointer inputImage)
  {
    m_InputImage = inputImage;
  }


  void SetThreshold(double threshold)
  {
    m_Threshold = threshold;
  }


protected:


  //CharImageType::Pointer m_Skeleton;
  CharImageType::Pointer m_RoiImg;
  ProjectionsImageType::Pointer m_Projections;
  //FloatImageType::Pointer m_MeanSkeleton;
  mitk::TbssImage::Pointer m_TbssImage;

  mitk::Image::Pointer m_InputImage;

  double m_Threshold;

  std::vector< itk::Index<3> > m_Path;

  std::string m_FileName;

  std::string m_FileNameLong; // For the regression analysis 'friendly' file

  std::vector< std::pair<std::string, int> > m_Groups;

  std::vector< itk::Index<3> > m_Roi;

private:

};

}

#endif //__itkTractAnalyzer_h_

