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

#include <mitkCoreObjectFactory.h>
#include "mitkImage.h"
#include <mitkLexicalCast.h>
#include <vnl/vnl_random.h>
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <itksys/SystemTools.hxx>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkProperties.h>

// ITK
#include <itkImageRegionIterator.h>

// MITK
#include <mitkIOUtil.h>

// Classification
#include <mitkCLUtil.h>
#include <mitkVigraRandomForestClassifier.h>

#include <QDir>
#include <QString>
#include <QStringList>

#include <itkLabelOverlapMeasuresImageFilter.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkNeighborhoodFunctorImageFilter.h>
#include <itkFirstOrderStatisticsFeatureFunctor.h>
#include <itkNeighborhood.h>
#include <itkHessianMatrixEigenvalueImageFilter.h>
#include <itkStructureTensorEigenvalueImageFilter.h>
#include <itkLineHistogramBasedMassImageFilter.h>

using namespace mitk;

std::vector<mitk::Image::Pointer> m_FeatureImageVector;

void ProcessFeatureImages(const mitk::Image::Pointer & raw_image, const mitk::Image::Pointer & brain_mask)
{
  typedef itk::Image<double,3> DoubleImageType;
  typedef itk::Image<short,3> ShortImageType;
  typedef itk::ConstNeighborhoodIterator<DoubleImageType> NeighborhoodType; // Neighborhood iterator to access image
  typedef itk::Functor::NeighborhoodFirstOrderStatistics<NeighborhoodType, double> FunctorType;
  typedef itk::NeighborhoodFunctorImageFilter<DoubleImageType, DoubleImageType, FunctorType> FOSFilerType;

  m_FeatureImageVector.clear();

  // RAW
  m_FeatureImageVector.push_back(raw_image);

  // GAUSS
  mitk::Image::Pointer smoothed;
  mitk::CLUtil::GaussianFilter(raw_image,smoothed,1);
  m_FeatureImageVector.push_back(smoothed);

  // Calculate Probability maps (parameters used from literatur)
  // CSF
  mitk::Image::Pointer csf_prob = mitk::Image::New();
  mitk::CLUtil::ProbabilityMap(smoothed,13.9, 8.3,csf_prob);
  m_FeatureImageVector.push_back(csf_prob);

  // Lesion

  mitk::Image::Pointer les_prob = mitk::Image::New();
  mitk::CLUtil::ProbabilityMap(smoothed,59, 11.6,les_prob);
  m_FeatureImageVector.push_back(les_prob);

  // Barin (GM/WM)

  mitk::Image::Pointer brain_prob = mitk::Image::New();
  mitk::CLUtil::ProbabilityMap(smoothed,32, 5.6,brain_prob);
  m_FeatureImageVector.push_back(brain_prob);

  std::vector<unsigned int> FOS_sizes;
  FOS_sizes.push_back(1);

  DoubleImageType::Pointer input;
  ShortImageType::Pointer mask;
  mitk::CastToItkImage(smoothed, input);
  mitk::CastToItkImage(brain_mask, mask);

  for(unsigned int i = 0 ; i < FOS_sizes.size(); i++)
  {
    FOSFilerType::Pointer filter = FOSFilerType::New();
    filter->SetNeighborhoodSize(FOS_sizes[i]);
    filter->SetInput(input);
    filter->SetMask(mask);

    filter->Update();
    FOSFilerType::DataObjectPointerArray array = filter->GetOutputs();

    for( unsigned int i = 0; i < FunctorType::OutputCount; i++)
    {
      mitk::Image::Pointer featureimage;
      mitk::CastToMitkImage(dynamic_cast<DoubleImageType *>(array[i].GetPointer()),featureimage);
      m_FeatureImageVector.push_back(featureimage);
      //      AddImageAsDataNode(featureimage,FunctorType::GetFeatureName(i))->SetVisibility(show_nodes);
    }
  }

  {
    itk::HessianMatrixEigenvalueImageFilter< DoubleImageType >::Pointer filter = itk::HessianMatrixEigenvalueImageFilter< DoubleImageType >::New();
    filter->SetInput(input);
    filter->SetImageMask(mask);
    filter->SetSigma(3);
    filter->Update();

    mitk::Image::Pointer o1,o2,o3;
    mitk::CastToMitkImage(filter->GetOutput(0),o1);
    mitk::CastToMitkImage(filter->GetOutput(1),o2);
    mitk::CastToMitkImage(filter->GetOutput(2),o3);

    m_FeatureImageVector.push_back(o1);
    m_FeatureImageVector.push_back(o2);
    m_FeatureImageVector.push_back(o3);
  }

  {
    itk::StructureTensorEigenvalueImageFilter< DoubleImageType >::Pointer filter = itk::StructureTensorEigenvalueImageFilter< DoubleImageType >::New();
    filter->SetInput(input);
    filter->SetImageMask(mask);
    filter->SetInnerScale(1.5);
    filter->SetOuterScale(3);
    filter->Update();

    mitk::Image::Pointer o1,o2,o3;
    mitk::CastToMitkImage(filter->GetOutput(0),o1);
    mitk::CastToMitkImage(filter->GetOutput(1),o2);
    mitk::CastToMitkImage(filter->GetOutput(2),o3);

    m_FeatureImageVector.push_back(o1);
    m_FeatureImageVector.push_back(o2);
    m_FeatureImageVector.push_back(o3);
  }

  {
    itk::LineHistogramBasedMassImageFilter< DoubleImageType >::Pointer filter = itk::LineHistogramBasedMassImageFilter< DoubleImageType >::New();
    filter->SetInput(input);
    filter->SetImageMask(mask);
    filter->Update();

    mitk::Image::Pointer o1;
    mitk::CastToMitkImage(filter->GetOutput(0),o1);

    m_FeatureImageVector.push_back(o1);
  }
}

std::vector<mitk::Point3D> PointSetToVector(const mitk::PointSet::Pointer & mps)
{
  std::vector<mitk::Point3D> result;
  for(int i = 0 ; i < mps->GetSize(); i++)
    result.push_back(mps->GetPoint(i));
  return result;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  // required params
  parser.addArgument("inputdir", "i", mitkCommandLineParser::Directory, "Input Directory", "Contains input feature files.", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("outputdir", "o", mitkCommandLineParser::Directory, "Output Directory", "Destination of output files.", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("mitkprojectdata", "d", mitkCommandLineParser::File, "original class mask and raw image", "Orig. data.", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("csfmps", "csf", mitkCommandLineParser::File, "CSF Pointset", ".", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("lesmps", "les", mitkCommandLineParser::File, "LES Pointset", ".", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("bramps", "bra", mitkCommandLineParser::File, "BRA Pointset", ".", us::Any(), false, false, false, mitkCommandLineParser::Input);
  //  parser.addArgument("points", "p", mitkCommandLineParser::Int, "Ensure that p points are selected", ".", us::Any(), false);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Evaluationtool for Manual-Segmentation");
  parser.setDescription("Uses Datacollection to calculate DICE scores for CSF LES BRA");
  parser.setContributor("MBI");

  // Params parsing
  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inputdir = us::any_cast<std::string>(parsedArgs["inputdir"]);
  std::string outputdir = us::any_cast<std::string>(parsedArgs["outputdir"]);
  std::string mitkprojectdata = us::any_cast<std::string>(parsedArgs["mitkprojectdata"]);
  std::string csf_mps_name = us::any_cast<std::string>(parsedArgs["csfmps"]);
  std::string les_mps_name = us::any_cast<std::string>(parsedArgs["lesmps"]);
  std::string bra_mps_name = us::any_cast<std::string>(parsedArgs["bramps"]);

  mitk::Image::Pointer class_mask_sampled, raw_image, class_mask;
  mitk::PointSet::Pointer CSF_mps, LES_mps, BRA_mps;

  // Load from mitk-project
  auto so = mitk::IOUtil::Load(inputdir + "/" + mitkprojectdata);
  std::map<unsigned int, unsigned int> map;
  mitk::CLUtil::CountVoxel(dynamic_cast<mitk::Image *>(so[1].GetPointer()), map);
  raw_image = map.size() <= 7 ? dynamic_cast<mitk::Image *>(so[0].GetPointer()) : dynamic_cast<mitk::Image *>(so[1].GetPointer());
  class_mask = map.size() <= 7 ? dynamic_cast<mitk::Image *>(so[1].GetPointer()) : dynamic_cast<mitk::Image *>(so[0].GetPointer());

  CSF_mps = mitk::IOUtil::Load<mitk::PointSet>(inputdir + "/" + csf_mps_name);
  LES_mps = mitk::IOUtil::Load<mitk::PointSet>(inputdir + "/" + les_mps_name);
  BRA_mps = mitk::IOUtil::Load<mitk::PointSet>(inputdir + "/" + bra_mps_name);

  unsigned int num_points = CSF_mps->GetSize() + LES_mps->GetSize() + BRA_mps->GetSize();
  MITK_INFO << "Found #" << num_points << " points over all classes.";

  ProcessFeatureImages(raw_image, class_mask);

  std::map<unsigned int, unsigned int> tmpMap;
  tmpMap[0] = 0;
  tmpMap[1] = 1;
  tmpMap[2] = 1;
  tmpMap[3] = 1;
  tmpMap[4] = 2;
  tmpMap[5] = 3;
  tmpMap[6] = 3;
  mitk::CLUtil::MergeLabels( class_mask, tmpMap);
  class_mask_sampled = class_mask->Clone();
  itk::Image<short,3>::Pointer itk_classmask_sampled;
  mitk::CastToItkImage(class_mask_sampled,itk_classmask_sampled);
  itk::ImageRegionIteratorWithIndex<itk::Image<short,3> >::IndexType index;
  itk::ImageRegionIteratorWithIndex<itk::Image<short,3> > iit(itk_classmask_sampled,itk_classmask_sampled->GetLargestPossibleRegion());

  std::ofstream myfile;
  myfile.open (inputdir + "/results_3.csv");

  Eigen::MatrixXd X_test;

  unsigned int count_test = 0;
  mitk::CLUtil::CountVoxel(class_mask, count_test);
  X_test = Eigen::MatrixXd(count_test, m_FeatureImageVector.size());

  unsigned int pos = 0;
  for( const auto & image : m_FeatureImageVector)
  {
    X_test.col(pos) = mitk::CLUtil::Transform<double>(image,class_mask);
    ++pos;
  }

  unsigned int runs = 20;
  for(unsigned int k = 0 ; k < runs; k++)
  {
    auto CSF_vec = PointSetToVector(CSF_mps);
    auto LES_vec = PointSetToVector(LES_mps);
    auto BRA_vec = PointSetToVector(BRA_mps);

    itk_classmask_sampled->FillBuffer(0);

    // initial draws
    std::random_shuffle(CSF_vec.begin(), CSF_vec.end());
    class_mask->GetGeometry()->WorldToIndex(CSF_vec.back(),index);
    iit.SetIndex(index);
    iit.Set(1);
    CSF_vec.pop_back();

    std::random_shuffle(LES_vec.begin(), LES_vec.end());
    class_mask->GetGeometry()->WorldToIndex(LES_vec.back(),index);
    iit.SetIndex(index);
    iit.Set(2);
    LES_vec.pop_back();

    std::random_shuffle(BRA_vec.begin(), BRA_vec.end());
    class_mask->GetGeometry()->WorldToIndex(BRA_vec.back(),index);
    iit.SetIndex(index);
    iit.Set(3);
    BRA_vec.pop_back();

    std::stringstream ss;

    while(!(CSF_vec.empty() && LES_vec.empty() && BRA_vec.empty()))
    {
      mitk::CastToMitkImage(itk_classmask_sampled, class_mask_sampled);

      // Train forest
      mitk::VigraRandomForestClassifier::Pointer classifier = mitk::VigraRandomForestClassifier::New();
      classifier->SetTreeCount(40);
      classifier->SetSamplesPerTree(0.66);

      Eigen::MatrixXd X_train;

      unsigned int count_train = 0;
      mitk::CLUtil::CountVoxel(class_mask_sampled, count_train);
      X_train = Eigen::MatrixXd(count_train, m_FeatureImageVector.size() );

      unsigned int pos = 0;
      for( const auto & image : m_FeatureImageVector)
      {
        X_train.col(pos) = mitk::CLUtil::Transform<double>(image,class_mask_sampled);
        ++pos;
      }

      Eigen::MatrixXi Y = mitk::CLUtil::Transform<int>(class_mask_sampled,class_mask_sampled);
      classifier->Train(X_train,Y);

      Eigen::MatrixXi Y_test = classifier->Predict(X_test);

      mitk::Image::Pointer result_mask = mitk::CLUtil::Transform<int>(Y_test, class_mask);
      itk::Image<short,3>::Pointer itk_result_mask, itk_class_mask;

      mitk::CastToItkImage(result_mask,itk_result_mask);
      mitk::CastToItkImage(class_mask, itk_class_mask);

      itk::LabelOverlapMeasuresImageFilter<itk::Image<short,3> >::Pointer overlap_filter = itk::LabelOverlapMeasuresImageFilter<itk::Image<short,3> >::New();
      overlap_filter->SetInput(0,itk_result_mask);
      overlap_filter->SetInput(1,itk_class_mask);
      overlap_filter->Update();

      MITK_INFO << "DICE (" << num_points - (CSF_vec.size() + LES_vec.size() + BRA_vec.size()) << "): " << overlap_filter->GetDiceCoefficient();
      ss << overlap_filter->GetDiceCoefficient() <<",";

      // random class selection

      if(!CSF_vec.empty())
      {
        std::random_shuffle(CSF_vec.begin(), CSF_vec.end());
        class_mask->GetGeometry()->WorldToIndex(CSF_vec.back(),index);
        iit.SetIndex(index);
        iit.Set(1);
        CSF_vec.pop_back();
      }

      if(!LES_vec.empty())
      {
        std::random_shuffle(LES_vec.begin(), LES_vec.end());
        class_mask->GetGeometry()->WorldToIndex(LES_vec.back(),index);
        iit.SetIndex(index);
        iit.Set(2);
        LES_vec.pop_back();
      }

      if(!BRA_vec.empty())
      {
        std::random_shuffle(BRA_vec.begin(), BRA_vec.end());
        class_mask->GetGeometry()->WorldToIndex(BRA_vec.back(),index);
        iit.SetIndex(index);
        iit.Set(3);
        BRA_vec.pop_back();
      }
    }

    myfile << ss.str() << "\n";
    myfile.flush();
  }

  myfile.close();

  return EXIT_SUCCESS;
}
