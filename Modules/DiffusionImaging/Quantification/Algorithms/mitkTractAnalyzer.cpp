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

#ifndef __mitkTractAnalyzer_cpp
#define __mitkTractAnalyzer_cpp


#include <mitkProgressBar.h>
#include <mitkLogMacros.h>
#include <mitkTractAnalyzer.h>

#include <itkImageRegionIteratorWithIndex.h>


#include <itkShortestPathCostFunctionTbss.h>
#include <itkShortestPathImageFilter.h>

#include <iostream>
#include <fstream>

namespace mitk {

  TractAnalyzer::TractAnalyzer() { }


  void TractAnalyzer::MakeRoi()
  {

    m_CostSum = 0.0;

    int n = 0;
    if(m_PointSetNode.IsNotNull())
    {
      n = m_PointSetNode->GetSize();
      if(n==0)
      {
        mitkThrow() << "No points have been set yet.";
      }
    }
    else{
      mitkThrow() << "No points have been set yet.";
    }

    std::string pathDescription = "";
    std::vector< itk::Index<3> > totalPath;

    if(n>0)
    {
      for(int i=0; i<n-1; ++i)
      {

        mitk::ProgressBar::GetInstance()->Progress();

        mitk::Point3D p = m_PointSetNode->GetPoint(i);
        mitk::Point3D p2 = m_PointSetNode->GetPoint(i+1);


        itk::Index<3> startPoint;
        itk::Index<3> endPoint;

        m_InputImage->GetGeometry()->WorldToIndex(p,startPoint);
        m_InputImage->GetGeometry()->WorldToIndex(p2,endPoint);

        MITK_INFO << "create roi";

        std::vector< itk::Index<3> > path = CreateSegment(startPoint, endPoint);

        for(auto it = path.begin();
            it != path.end(); it++)
        {
          itk::Index<3> ix = *it;

          if (!(ix==endPoint))
          {
            mitk::ProgressBar::GetInstance()->Progress();

            totalPath.push_back(ix);
            std::stringstream ss;
            ss << ix[0] << " " << ix[1] << " " << ix[2] << "\n";
            pathDescription += ss.str();
          }
          else
          {
            // Only when dealing with the last segment the last point should be added. This one will not occur
            // as the first point of the next roi segment.
            if(i == (n-2))
            {
              totalPath.push_back(endPoint);
              std::stringstream ss;
              ss << endPoint[0] << " " << endPoint[1] << " " << endPoint[2] << "\n";
              pathDescription += ss.str();
            }

          }

        }

      }


      // save pathDescription to m_PathDescription
      m_PathDescription = pathDescription;

      FloatImageType::Pointer itkImg = FloatImageType::New();
      mitk::CastToItkImage(m_InputImage, itkImg);

      CharImageType::Pointer roiImg = CharImageType::New();
      roiImg->SetRegions(itkImg->GetLargestPossibleRegion().GetSize());
      roiImg->SetOrigin(itkImg->GetOrigin());
      roiImg->SetSpacing(itkImg->GetSpacing());
      roiImg->SetDirection(itkImg->GetDirection());
      roiImg->Allocate();
      roiImg->FillBuffer(0);


      std::vector< itk::Index<3> > roi;

      std::vector< itk::Index<3> >::iterator it;
      for(it = totalPath.begin();
          it != totalPath.end();
          it++)
      {
        itk::Index<3> ix = *it;
        roiImg->SetPixel(ix, 1);
        roi.push_back(ix);
      }


      m_TbssRoi = mitk::TbssRoiImage::New();

      m_TbssRoi->SetRoi(roi);

      m_TbssRoi->SetImage(roiImg);

      m_TbssRoi->InitializeFromImage();



    }



  }



  std::vector< itk::Index<3> > TractAnalyzer::CreateSegment(itk::Index<3> startPoint, itk::Index<3> endPoint)
  {

      typedef itk::ShortestPathImageFilter<FloatImageType, CharImageType> ShortestPathFilterType;
      typedef itk::ShortestPathCostFunctionTbss<FloatImageType> CostFunctionType;


      FloatImageType::Pointer meanSkeleton;

      mitk::CastToItkImage(m_InputImage, meanSkeleton);

      // Only use the mitk image



      if(meanSkeleton)
      {
        CostFunctionType::Pointer costFunction = CostFunctionType::New();
        costFunction->SetImage(meanSkeleton);
        costFunction->SetStartIndex(startPoint);
        costFunction->SetEndIndex(endPoint);
        costFunction->SetThreshold(m_Threshold);

        ShortestPathFilterType::Pointer pathFinder = ShortestPathFilterType::New();
        pathFinder->SetCostFunction(costFunction);
        pathFinder->SetFullNeighborsMode(true);
        pathFinder->SetGraph_fullNeighbors(true);
        //pathFinder->SetCalcMode(ShortestPathFilterType::A_STAR);
        pathFinder->SetInput(meanSkeleton);
        pathFinder->SetStartIndex(startPoint);
        pathFinder->SetEndIndex(endPoint);
        pathFinder->Update();

        double segmentCost = 0.0;
        std::vector< itk::Index<3> > path = pathFinder->GetVectorPath();

        for(unsigned int i=0; i<path.size()-1; i++)
        {
            itk::Index<3> ix1 = path[i];
            itk::Index<3> ix2 = path[i+1];

            segmentCost += costFunction->GetCost(ix1, ix2);
        }

        m_CostSum += segmentCost;


        return pathFinder->GetVectorPath();


      }
      return std::vector< itk::Index<3> >();
  }


}
#endif
