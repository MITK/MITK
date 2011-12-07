
#ifndef __mitkTractAnalyzer_cpp
#define __mitkTractAnalyzer_cpp



#include <mitkLogMacros.h>
#include <mitkTractAnalyzer.h>

#include <itkImageRegionIteratorWithIndex.h>


#include <itkShortestPathCostFunctionTbss.h>
#include <itkShortestPathImageFilter.h>

#include <iostream>
#include <fstream>


using namespace std;

namespace mitk {

  TractAnalyzer::TractAnalyzer()
  {

  }


  void TractAnalyzer::BuildGraph(itk::Index<3> startPoint, itk::Index<3> endPoint)
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
      //pathFinder->SetCalcMode(ShortestPathFilterType::A_STAR);
      pathFinder->SetInput(meanSkeleton);
      pathFinder->SetStartIndex(startPoint);
      pathFinder->SetEndIndex(endPoint);
      pathFinder->Update();

      m_Path = pathFinder->GetVectorPath();


      m_RoiImg = pathFinder->GetOutput();
    }



  }

  void TractAnalyzer::MeasureRoi()
  {

    // Output two types
    ProjectionsImageType::SizeType size = m_Projections->GetLargestPossibleRegion().GetSize();

    std::ofstream file(m_FileName.c_str());

    std::vector<std::string> individuals;
    for(int i=0; i<m_Groups.size(); i++)
    {
      std::pair<std::string, int> group = m_Groups[i];
      for(int j=0; j<group.second; j++)
      {
        individuals.push_back(group.first);
      }
    }


    // Position ID

    file << "position ";

    for(int i=0; i<m_Roi.size(); i++)
    {
      file << "pos" << i << " ";
    }

    file << "\n";

    // For every subect
    for(int j=0; j<size[3]; j++)
    {

      file << individuals[j] << " ";

      // For every point on the current ROI
      for(int k=0; k<m_Roi.size(); k++)
      {
        itk::Index<3> ix = m_Roi[k];
        itk::Index<4> ix4;
        ix4[0] = ix[0];
        ix4[1] = ix[1];
        ix4[2] = ix[2];
        ix4[3] = j;

        float f = m_Projections->GetPixel(ix4);

        file << f << " ";

      }

      file << "\n";

    }

    file.close();



    // Write the long format output
    std::ofstream fileLong(m_FileNameLong.c_str());

    fileLong << "ID " << "group " << "position " << "value\n";

    for(int i=0; i<m_Roi.size(); i++)
    {
      for (int j=0; j<individuals.size(); j++)
      {
        itk::Index<3> ix = m_Roi[i];
        itk::Index<4> ix4;
        ix4[0] = ix[0];
        ix4[1] = ix[1];
        ix4[2] = ix[2];
        ix4[3] = j;

        float f = m_Projections->GetPixel(ix4);

        fileLong << "ID" << j << " " << individuals[j] << " " << "pos" << i << " "<< f << "\n";
      }
    }

    fileLong.close();


  }



}
#endif
