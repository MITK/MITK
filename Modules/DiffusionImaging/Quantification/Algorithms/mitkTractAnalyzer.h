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

#include "QuantificationExports.h"
#include <itkImage.h>
#include <itkVectorImage.h>
#include "mitkImage.h"
#include "mitkImageCast.h"

#include <mitkTbssRoiImage.h>
#include <mitkPointSet.h>


namespace mitk{
/** \class TractAnalyzer
 */


class Quantification_EXPORT TractAnalyzer
{

public:


  TractAnalyzer();
  ~TractAnalyzer() {};

  typedef itk::Image<char,3> CharImageType;
  typedef itk::Image<float,3> FloatImageType;
  typedef itk::Image<float,4> ProjectionsImageType;
  typedef itk::VectorImage<float, 3> VectorImageType;

  void BuildGraph();


 /*
  std::vector< itk::Index<3> > GetPath()
  {
    return m_Path;
  }


  CharImageType::Pointer GetRoiImage()
  {
    return m_RoiImg;
  }



*/

  mitk::TbssRoiImage::Pointer GetRoiImage()
  {
    return m_TbssRoi;
  }


  void SetInputImage(mitk::Image::Pointer inputImage)
  {
    m_InputImage = inputImage;
  }

  void SetPointSet(mitk::PointSet::Pointer pointSet)
  {
    m_PointSetNode = pointSet;
  }


  void SetThreshold(double threshold)
  {
    m_Threshold = threshold;
  }

  std::string GetPathDescription()
  {
    return m_PathDescription;
  }

protected:



  std::vector< itk::Index<3> > CreateSegment(itk::Index<3> startPoint, itk::Index<3> endPoint);


  mitk::TbssRoiImage::Pointer m_TbssRoi;

  //CharImageType::Pointer m_RoiImg;

  mitk::Image::Pointer m_InputImage;

  double m_Threshold;

  //std::vector< itk::Index<3> > m_Path;

  mitk::PointSet::Pointer m_PointSetNode;

  std::string m_PathDescription;



private:

};

}

#endif //__itkTractAnalyzer_h_
