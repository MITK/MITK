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

#include "MiniAppManager.h"
#include <mitkImageCast.h>
#include <mitkDiffusionImage.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkIOUtil.h>
#include <mitkNrrdDiffusionImageWriter.h>
#include <mitkFiberBundleX.h>
//#include "ctkCommandLineParser.h"
#include "ctkCommandLineParser.h"
#include <mitkRicianNoiseModel.h>

#include <itkAddArtifactsToDwiImageFilter.h>
#include <itkTractsToDWIImageFilter.h>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include "boost/foreach.hpp"
#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkAstroStickModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>

using namespace mitk;

struct ImageParameters {
  itk::ImageRegion<3>                 imageRegion;
  itk::Vector<double,3>               imageSpacing;
  itk::Point<double,3>                imageOrigin;
  itk::Matrix<double, 3, 3>           imageDirection;
  unsigned int                        numGradients;
  double                              b_value;
  unsigned int                        repetitions;
  double                              signalScale;
  double                              tEcho;
  double                              tLine;
  double                              tInhom;
  double                              axonRadius;
  unsigned int                        interpolationShrink;
  double                              kspaceLineOffset;
  double                              upsampling;
  double                              eddyStrength;
  double                              comp3Weight;
  double                              comp4Weight;
  int                                 spikes;
  double                              spikeAmplitude;

  bool                                doSimulateRelaxation;
  bool                                doSimulateEddyCurrents;
  bool                                doDisablePartialVolume;

  mitk::RicianNoiseModel<double>       ricianNoiseModel;
  mitk::DiffusionSignalModel<double>::GradientListType  gradientDirections;
  itk::TractsToDWIImageFilter< short >::DiffusionModelList fiberModelList, nonFiberModelList;
  itk::TractsToDWIImageFilter< short >::KspaceArtifactList artifactList;
  std::string signalModelString, artifactModelString;

  itk::Image<double, 3>::Pointer           frequencyMap;
  itk::Image<unsigned char, 3>::Pointer    tissueMaskImage;

  mitk::DataNode::Pointer             resultNode;
};



void LoadParameters(const std::string & filename,
                    ImageParameters  & m_ImageGenParameters,
                    mitk::Image::Pointer m_fImage,
                    mitk::Image::Pointer m_maskImage,
                    mitk::StickModel<double> * m_StickModel1,
                    mitk::StickModel<double> * m_StickModel2,
                    mitk::TensorModel<double> * m_ZeppelinModel1,
                    mitk::TensorModel<double> * m_ZeppelinModel2,
                    mitk::TensorModel<double> * m_TensorModel1,
                    mitk::TensorModel<double> * m_TensorModel2,

                    // extra axonal compartment models
                    mitk::BallModel<double> * m_BallModel1,
                    mitk::BallModel<double> * m_BallModel2,
                    mitk::AstroStickModel<double> * m_AstrosticksModel1,
                    mitk::AstroStickModel<double> * m_AstrosticksModel2,
                    mitk::DotModel<double> * m_DotModel1,
                    mitk::DotModel<double> * m_DotModel2)
{



  MITK_INFO << "Initialize Diffusion Models";

  boost::property_tree::ptree parameters;
  boost::property_tree::xml_parser::read_xml(filename, parameters);

  m_ImageGenParameters.artifactList.clear();
  m_ImageGenParameters.nonFiberModelList.clear();
  m_ImageGenParameters.fiberModelList.clear();
  m_ImageGenParameters.signalModelString = "";
  m_ImageGenParameters.artifactModelString = "";
  m_ImageGenParameters.resultNode = mitk::DataNode::New();
  //m_ImageGenParameters.tissueMaskImage = NULL;
  //m_ImageGenParameters.frequencyMap = NULL;
  //m_ImageGenParameters.gradientDirections.clear();
  m_ImageGenParameters.spikes = 0;
  m_ImageGenParameters.spikeAmplitude = 1;

  MITK_INFO << "reset params";

  BOOST_FOREACH( boost::property_tree::ptree::value_type const& v1, parameters.get_child("fiberfox") )
  {
    if( v1.first == "image" )
    {
      MITK_INFO << "Load image params";
      m_ImageGenParameters.tEcho = v1.second.get<double>("tEcho");
      m_ImageGenParameters.tLine = v1.second.get<double>("tLine");

      m_ImageGenParameters.doSimulateEddyCurrents = v1.second.get<bool>("artifacts.addeddy");
      m_ImageGenParameters.eddyStrength = 0;
      if (m_ImageGenParameters.doSimulateEddyCurrents)
      {
        m_ImageGenParameters.eddyStrength = v1.second.get<double>("artifacts.eddyStrength");
      }

      // signal relaxation
      m_ImageGenParameters.doSimulateRelaxation = v1.second.get<bool>("doSimulateRelaxation");
      if (m_ImageGenParameters.doSimulateRelaxation)

        // N/2 ghosts
        if (v1.second.get<bool>("artifacts.addghost"))
        {
          m_ImageGenParameters.kspaceLineOffset = v1.second.get<double>("artifacts.kspaceLineOffset");
        }
        else
        {
          m_ImageGenParameters.kspaceLineOffset = 0;
        }

      if (v1.second.get<bool>("artifacts.addspikes"))
      {
        m_ImageGenParameters.spikes = v1.second.get<int>("artifacts.spikesnum");
        m_ImageGenParameters.spikeAmplitude = v1.second.get<double>("artifacts.spikesscale");
      }


      // add distortions
      if (v1.second.get<bool>("artifacts.distortions") && m_fImage)
      {
        itk::Image<double,3>::Pointer itkImg = itk::Image<double,3>::New();
        mitk::CastToItkImage< itk::Image<double,3> >(m_fImage, itkImg);

        if (m_ImageGenParameters.imageRegion.GetSize(0)==itkImg->GetLargestPossibleRegion().GetSize(0) &&
            m_ImageGenParameters.imageRegion.GetSize(1)==itkImg->GetLargestPossibleRegion().GetSize(1) &&
            m_ImageGenParameters.imageRegion.GetSize(2)==itkImg->GetLargestPossibleRegion().GetSize(2))
        {
          m_ImageGenParameters.frequencyMap = itkImg;
        }
      }

      // rician noise
      if (v1.second.get<bool>("artifacts.addnoise"))
        m_ImageGenParameters.ricianNoiseModel.SetNoiseVariance(v1.second.get<double>("artifacts.noisevariance"));
      else
        m_ImageGenParameters.ricianNoiseModel.SetNoiseVariance(0);

      // gibbs ringing
      m_ImageGenParameters.upsampling = 1;
      if (v1.second.get<bool>("artifacts.addringing"))
        m_ImageGenParameters.upsampling = v1.second.get<double>("artifacts.ringingupsampling");

      // adjusting line readout time to the adapted image size needed for the DFT
      int y = m_ImageGenParameters.imageRegion.GetSize(1);
      if ( y%2 == 1 )
        y += 1;
      if ( y>m_ImageGenParameters.imageRegion.GetSize(1) )
        m_ImageGenParameters.tLine *= (double)m_ImageGenParameters.imageRegion.GetSize(1)/y;


      // check tissue mask
      if (m_maskImage.IsNotNull())
      {
        m_ImageGenParameters.tissueMaskImage = itk::Image<unsigned char,3>::New();
        mitk::CastToItkImage<itk::Image<unsigned char,3> >(m_maskImage.GetPointer(), m_ImageGenParameters.tissueMaskImage);
      }

      // signal models
      m_ImageGenParameters.comp3Weight = 1;
      m_ImageGenParameters.comp4Weight = 0;
      if (v1.second.get<int>("compartment4.index") > 0)
      {
        m_ImageGenParameters.comp4Weight = v1.second.get<double>("compartment4.weight");
        m_ImageGenParameters.comp3Weight -= m_ImageGenParameters.comp4Weight;
      }



      // compartment 1
      switch(v1.second.get<int>("compartment1.index")){
      case 0:
        m_StickModel1->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_StickModel1->SetBvalue(m_ImageGenParameters.b_value);
        m_StickModel1->SetDiffusivity(v1.second.get<double>("compartment1.stick.d"));
        m_StickModel1->SetT2(v1.second.get<double>("compartment1.stick.t2"));
        m_ImageGenParameters.fiberModelList.push_back(m_StickModel1);
        break;
      case 1:
        m_ZeppelinModel1->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_ZeppelinModel1->SetBvalue(m_ImageGenParameters.b_value);
        m_ZeppelinModel1->SetDiffusivity1(v1.second.get<double>("compartment1.zeppelin.d1"));
        m_ZeppelinModel1->SetDiffusivity2(v1.second.get<double>("compartment1.zeppelin.d2"));
        m_ZeppelinModel1->SetDiffusivity3(v1.second.get<double>("compartment1.zeppelin.d2"));
        m_ZeppelinModel1->SetT2(v1.second.get<double>("compartment1.zeppelin.t2"));
        m_ImageGenParameters.fiberModelList.push_back(m_ZeppelinModel1);
        break;
      case 2:
        m_TensorModel1->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_TensorModel1->SetBvalue(m_ImageGenParameters.b_value);
        m_TensorModel1->SetDiffusivity1(v1.second.get<double>("compartment1.tensor.d1"));
        m_TensorModel1->SetDiffusivity2(v1.second.get<double>("compartment1.tensor.d2"));
        m_TensorModel1->SetDiffusivity3(v1.second.get<double>("compartment1.tensor.d3"));
        m_TensorModel1->SetT2(v1.second.get<double>("compartment1.tensor.t2"));
        m_ImageGenParameters.fiberModelList.push_back(m_TensorModel1);
        break;
      }

      // compartment 2
      switch(v1.second.get<int>("compartment2.index")){
      case 0:
        m_StickModel2->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_StickModel2->SetBvalue(m_ImageGenParameters.b_value);
        m_StickModel2->SetDiffusivity(v1.second.get<double>("compartment2.stick.d"));
        m_StickModel2->SetT2(v1.second.get<double>("compartment2.stick.t2"));
        m_ImageGenParameters.fiberModelList.push_back(m_StickModel2);
        break;
      case 1:
        m_ZeppelinModel2->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_ZeppelinModel2->SetBvalue(m_ImageGenParameters.b_value);
        m_ZeppelinModel2->SetDiffusivity1(v1.second.get<double>("compartment2.zeppelin.d1"));
        m_ZeppelinModel2->SetDiffusivity2(v1.second.get<double>("compartment2.zeppelin.d2"));
        m_ZeppelinModel2->SetDiffusivity3(v1.second.get<double>("compartment2.zeppelin.d2"));
        m_ZeppelinModel2->SetT2(v1.second.get<double>("compartment2.zeppelin.t2"));
        m_ImageGenParameters.fiberModelList.push_back(m_ZeppelinModel2);
        break;
      case 2:
        m_TensorModel2->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_TensorModel2->SetBvalue(m_ImageGenParameters.b_value);
        m_TensorModel2->SetDiffusivity1(v1.second.get<double>("compartment2.tensor.d1"));
        m_TensorModel2->SetDiffusivity2(v1.second.get<double>("compartment2.tensor.d2"));
        m_TensorModel2->SetDiffusivity3(v1.second.get<double>("compartment2.tensor.d3"));
        m_TensorModel2->SetT2(v1.second.get<double>("compartment2.tensor.t2"));
        m_ImageGenParameters.fiberModelList.push_back(m_TensorModel2);
        break;
      }

      // compartment 3
      switch(v1.second.get<int>("compartment3.index")){
      case 0:
        m_BallModel1->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_BallModel1->SetBvalue(m_ImageGenParameters.b_value);
        m_BallModel1->SetDiffusivity(v1.second.get<double>("compartment3.ball.d"));
        m_BallModel1->SetT2(v1.second.get<double>("compartment3.ball.t2"));
        m_BallModel1->SetWeight(m_ImageGenParameters.comp3Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(m_BallModel1);
        break;
      case 1:
        m_AstrosticksModel1->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_AstrosticksModel1->SetBvalue(m_ImageGenParameters.b_value);
        m_AstrosticksModel1->SetDiffusivity(v1.second.get<double>("compartment3.astrosticks.d"));
        m_AstrosticksModel1->SetT2(v1.second.get<double>("compartment3.astrosticks.t2"));
        m_AstrosticksModel1->SetRandomizeSticks(v1.second.get<bool>("compartment3.astrosticks.randomize"));
        m_AstrosticksModel1->SetWeight(m_ImageGenParameters.comp3Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(m_AstrosticksModel1);
        break;
      case 2:
        m_DotModel1->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_DotModel1->SetT2(v1.second.get<double>("compartment3.dot.t2"));
        m_DotModel1->SetWeight(m_ImageGenParameters.comp3Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(m_DotModel1);
        break;
      }

      // compartment 4
      switch(v1.second.get<int>("compartment4.index")){
      case 0:
        m_BallModel2->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_BallModel2->SetBvalue(m_ImageGenParameters.b_value);
        m_BallModel2->SetDiffusivity(v1.second.get<double>("compartment4.ball.d"));
        m_BallModel2->SetT2(v1.second.get<double>("compartment4.ball.t2"));
        m_BallModel2->SetWeight(m_ImageGenParameters.comp4Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(m_BallModel2);
        break;
      case 1:
        m_AstrosticksModel2->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_AstrosticksModel2->SetBvalue(m_ImageGenParameters.b_value);
        m_AstrosticksModel2->SetDiffusivity(v1.second.get<double>("compartment4.astrosticks.d"));
        m_AstrosticksModel2->SetT2(v1.second.get<double>("compartment4.astrosticks.t2"));
        m_AstrosticksModel2->SetRandomizeSticks(v1.second.get<bool>("compartment4.astrosticks.randomize"));
        m_AstrosticksModel2->SetWeight(m_ImageGenParameters.comp4Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(m_AstrosticksModel2);
        break;
      case 2:
        m_DotModel2->SetGradientList(m_ImageGenParameters.gradientDirections);
        m_DotModel2->SetT2(v1.second.get<double>("compartment4.dot.t2"));
        m_DotModel2->SetWeight(m_ImageGenParameters.comp4Weight);
        m_ImageGenParameters.nonFiberModelList.push_back(m_DotModel2);
        break;
      }

      m_ImageGenParameters.signalScale = v1.second.get<int>("signalScale");
      m_ImageGenParameters.repetitions = v1.second.get<int>("repetitions");

      m_ImageGenParameters.tInhom = v1.second.get<double>("tInhom");

      m_ImageGenParameters.doDisablePartialVolume = v1.second.get<bool>("doDisablePartialVolume");
      m_ImageGenParameters.interpolationShrink = v1.second.get<int>("interpolationShrink");
      m_ImageGenParameters.axonRadius = v1.second.get<double>("axonRadius");
    }

    /*
m_Controls->m_VarianceBox->setValue(v1.second.get<double>("variance"));



m_Controls->m_AdvancedOptionsBox->setChecked(v1.second.get<bool>("showadvanced"));
m_Controls->m_AdvancedOptionsBox_2->setChecked(v1.second.get<bool>("showadvanced"));

            m_Controls->m_VolumeFractionsBox->setChecked(v1.second.get<bool>("outputvolumefractions"));
            m_Controls->m_RealTimeFibers->setChecked(v1.second.get<bool>("realtime"));

            m_Controls->m_DistributionBox->setCurrentIndex(v1.second.get<int>("distribution"));

            m_Controls->m_FiberDensityBox->setValue(v1.second.get<int>("density"));
            m_Controls->m_IncludeFiducials->setChecked(v1.second.get<bool>("includeFiducials"));
            m_Controls->m_ConstantRadiusBox->setChecked(v1.second.get<bool>("constantradius"));




            BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second )
            {
                if( v2.first == "spline" )
                {
                    m_Controls->m_FiberSamplingBox->setValue(v2.second.get<double>("sampling"));
                    m_Controls->m_TensionBox->setValue(v2.second.get<double>("tension"));
                    m_Controls->m_ContinuityBox->setValue(v2.second.get<double>("continuity"));
                    m_Controls->m_BiasBox->setValue(v2.second.get<double>("bias"));
                }
                if( v2.first == "rotation" )
                {
                    m_Controls->m_XrotBox->setValue(v2.second.get<double>("x"));
                    m_Controls->m_YrotBox->setValue(v2.second.get<double>("y"));
                    m_Controls->m_ZrotBox->setValue(v2.second.get<double>("z"));
                }
                if( v2.first == "translation" )
                {
                    m_Controls->m_XtransBox->setValue(v2.second.get<double>("x"));
                    m_Controls->m_YtransBox->setValue(v2.second.get<double>("y"));
                    m_Controls->m_ZtransBox->setValue(v2.second.get<double>("z"));
                }
                if( v2.first == "scale" )
                {
                    m_Controls->m_XscaleBox->setValue(v2.second.get<double>("x"));
                    m_Controls->m_YscaleBox->setValue(v2.second.get<double>("y"));
                    m_Controls->m_ZscaleBox->setValue(v2.second.get<double>("z"));
                }
            }
        }
        if( v1.first == "image" )
        {
            m_Controls->m_SizeX->setValue(v1.second.get<int>("basic.size.x"));
            m_Controls->m_SizeY->setValue(v1.second.get<int>("basic.size.y"));
            m_Controls->m_SizeZ->setValue(v1.second.get<int>("basic.size.z"));
            m_Controls->m_SpacingX->setValue(v1.second.get<double>("basic.spacing.x"));
            m_Controls->m_SpacingY->setValue(v1.second.get<double>("basic.spacing.y"));
            m_Controls->m_SpacingZ->setValue(v1.second.get<double>("basic.spacing.z"));
            m_Controls->m_NumGradientsBox->setValue(v1.second.get<int>("basic.numgradients"));
            m_Controls->m_BvalueBox->setValue(v1.second.get<int>("basic.bvalue"));
*/
  }

}


int FiberFoxProcessing(int argc, char* argv[])
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("in", "i", ctkCommandLineParser::String, "input file", us::Any(), false);
  parser.addArgument("out", "o", ctkCommandLineParser::String, "output file", us::Any(), false);
  parser.addArgument("fiberbundle", "f", ctkCommandLineParser::String, "defined fiber bundle for signal generation", us::Any(), false);
  parser.addArgument("loadparameters", "l", ctkCommandLineParser::String, "load fiber fox signal parameter file", us::Any(), false);


  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  string inName = us::any_cast<string>(parsedArgs["in"]);
  string outName = us::any_cast<string>(parsedArgs["out"]);
  string fbName = us::any_cast<string>(parsedArgs["fiberbundle"]);
  string paramName = us::any_cast<string>(parsedArgs["loadparameters"]);

  {
    RegisterDiffusionCoreObjectFactory();
    RegisterFiberTrackingObjectFactory();

    ImageParameters m_ImageGenParameters;
    mitk::Image::Pointer m_maskImage = 0;
    mitk::Image::Pointer m_fImage = 0;

    MITK_INFO << "Loading " << inName;
    const std::string s1="", s2="";
    std::vector<BaseData::Pointer> infile = BaseDataIO::LoadBaseDataFromFile( inName, s1, s2, false );
    mitk::BaseData::Pointer baseData = infile.at(0);

    MITK_INFO << "Loading " << fbName;
    std::vector<BaseData::Pointer> infile2 = BaseDataIO::LoadBaseDataFromFile( fbName, s1, s2, false );
    mitk::BaseData::Pointer baseData2 = infile2.at(0);

    DiffusionImage<short>::Pointer dwi;
    FiberBundleX::Pointer fbi;

    if ( dynamic_cast<DiffusionImage<short>*>(baseData.GetPointer()) )
      dwi = dynamic_cast<DiffusionImage<short>*>(baseData.GetPointer());
    else
      MITK_ERROR << "LOADING DWI FAILD: " << inName;

    if ( dynamic_cast<FiberBundleX*>(baseData2.GetPointer()) )
      fbi = dynamic_cast<FiberBundleX*>(baseData2.GetPointer());
    else
      MITK_ERROR << "LOADING FBI FAILD: " << fbName;



    m_ImageGenParameters.imageRegion = dwi->GetVectorImage()->GetLargestPossibleRegion();
    m_ImageGenParameters.imageSpacing = dwi->GetVectorImage()->GetSpacing();
    m_ImageGenParameters.imageOrigin = dwi->GetVectorImage()->GetOrigin();
    m_ImageGenParameters.imageDirection = dwi->GetVectorImage()->GetDirection();
    m_ImageGenParameters.b_value = dwi->GetB_Value();
    mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer dirs = dwi->GetDirections();

    m_ImageGenParameters.numGradients = 0;
    for (int i=0; i<dirs->Size(); i++)
    {
      DiffusionSignalModel<double>::GradientType g;
      g[0] = dirs->at(i)[0];
      g[1] = dirs->at(i)[1];
      g[2] = dirs->at(i)[2];
      m_ImageGenParameters.gradientDirections.push_back(g);
      if (dirs->at(i).magnitude()>0.0001)
        m_ImageGenParameters.numGradients++;
    }

    mitk::StickModel<double> m_StickModel1;
    mitk::StickModel<double> m_StickModel2;
    mitk::TensorModel<double> m_ZeppelinModel1;
    mitk::TensorModel<double> m_ZeppelinModel2;
    mitk::TensorModel<double> m_TensorModel1;
    mitk::TensorModel<double> m_TensorModel2;

    // extra axonal compartment models
    mitk::BallModel<double> m_BallModel1;
    mitk::BallModel<double> m_BallModel2;
    mitk::AstroStickModel<double> m_AstrosticksModel1;
    mitk::AstroStickModel<double> m_AstrosticksModel2;
    mitk::DotModel<double> m_DotModel1;
    mitk::DotModel<double> m_DotModel2;

    LoadParameters(paramName,m_ImageGenParameters,NULL, NULL,
                   &m_StickModel1,
                   &m_StickModel2,
                   &m_ZeppelinModel1,
                   &m_ZeppelinModel2,
                   &m_TensorModel1,
                   &m_TensorModel2,
                   &m_BallModel1,
                   &m_BallModel2,
                   &m_AstrosticksModel1,
                   &m_AstrosticksModel2,
                   &m_DotModel1,
                   &m_DotModel2);

    MITK_INFO << "Parameter loaded";

    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();

    tractsToDwiFilter->SetSimulateEddyCurrents(m_ImageGenParameters.doSimulateEddyCurrents);
    tractsToDwiFilter->SetEddyGradientStrength(m_ImageGenParameters.eddyStrength);
    //tractsToDwiFilter->SetUpsampling(m_ImageGenParameters.upsampling);
    tractsToDwiFilter->SetSimulateRelaxation(m_ImageGenParameters.doSimulateRelaxation);
    tractsToDwiFilter->SetImageRegion(m_ImageGenParameters.imageRegion);
    tractsToDwiFilter->SetSpacing(m_ImageGenParameters.imageSpacing);
    tractsToDwiFilter->SetOrigin(m_ImageGenParameters.imageOrigin);
    tractsToDwiFilter->SetDirectionMatrix(m_ImageGenParameters.imageDirection);
    tractsToDwiFilter->SetFiberBundle(fbi);
    tractsToDwiFilter->SetFiberModels(m_ImageGenParameters.fiberModelList);
    tractsToDwiFilter->SetNonFiberModels(m_ImageGenParameters.nonFiberModelList);
    tractsToDwiFilter->SetNoiseModel(&m_ImageGenParameters.ricianNoiseModel);
    tractsToDwiFilter->SetKspaceArtifacts(m_ImageGenParameters.artifactList);
    tractsToDwiFilter->SetkOffset(m_ImageGenParameters.kspaceLineOffset);
    tractsToDwiFilter->SettLine(m_ImageGenParameters.tLine);
    tractsToDwiFilter->SettInhom(m_ImageGenParameters.tInhom);
    tractsToDwiFilter->SetTE(m_ImageGenParameters.tEcho);
    tractsToDwiFilter->SetNumberOfRepetitions(m_ImageGenParameters.repetitions);
    tractsToDwiFilter->SetEnforcePureFiberVoxels(m_ImageGenParameters.doDisablePartialVolume);
    tractsToDwiFilter->SetInterpolationShrink(m_ImageGenParameters.interpolationShrink);
    tractsToDwiFilter->SetFiberRadius(m_ImageGenParameters.axonRadius);
    tractsToDwiFilter->SetSignalScale(m_ImageGenParameters.signalScale);
    if (m_ImageGenParameters.interpolationShrink>0)
      tractsToDwiFilter->SetUseInterpolation(true);
    tractsToDwiFilter->SetTissueMask(m_ImageGenParameters.tissueMaskImage);
    tractsToDwiFilter->SetFrequencyMap(m_ImageGenParameters.frequencyMap);
    tractsToDwiFilter->SetSpikeAmplitude(m_ImageGenParameters.spikeAmplitude);
    tractsToDwiFilter->SetSpikes(m_ImageGenParameters.spikes);
    tractsToDwiFilter->Update();

    mitk::DiffusionImage<short>::Pointer image = mitk::DiffusionImage<short>::New();
    image->SetVectorImage( tractsToDwiFilter->GetOutput() );
    image->SetB_Value(dwi->GetB_Value());
    image->SetDirections(dwi->GetDirections());
    image->InitializeFromVectorImage();

    MITK_INFO << "Writing " << outName;
    NrrdDiffusionImageWriter<short>::Pointer writer = NrrdDiffusionImageWriter<short>::New();
    writer->SetFileName(outName);
    writer->SetInput(image);
    writer->Update();


  }
  MITK_INFO << "DONE";
  return EXIT_SUCCESS;
}
RegisterDiffusionMiniApp(FiberFoxProcessing);
