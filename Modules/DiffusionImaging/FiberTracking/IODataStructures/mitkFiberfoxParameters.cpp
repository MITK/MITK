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

#define RAPIDXML_NO_EXCEPTIONS
#include <boost/foreach.hpp>
#include <mitkLexicalCast.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <mitkLog.h>
#include <algorithm>
#include <string>
#include <mitkFiberfoxParameters.h>
#include <mitkDiffusionFunctionCollection.h>

mitk::FiberfoxParameters::FiberfoxParameters()
  : m_NoiseModel(nullptr)
{
  mitk::StickModel<ScalarType>* model_aniso = new mitk::StickModel<ScalarType>();
  model_aniso->m_CompartmentId = 1;
  m_FiberModelList.push_back(model_aniso);

  mitk::BallModel<ScalarType>* model_iso = new mitk::BallModel<ScalarType>();
  model_iso->m_CompartmentId = 3;
  m_NonFiberModelList.push_back(model_iso);
}

mitk::FiberfoxParameters::FiberfoxParameters(const mitk::FiberfoxParameters& params)
  : m_NoiseModel(nullptr)
{
  m_FiberGen = params.m_FiberGen;
  m_SignalGen = params.m_SignalGen;
  m_Misc = params.m_Misc;

  if (params.m_NoiseModel!=nullptr)
  {
    if (dynamic_cast<mitk::RicianNoiseModel<>*>(params.m_NoiseModel.get()))
      m_NoiseModel = std::make_shared< mitk::RicianNoiseModel<> >();
    else if (dynamic_cast<mitk::ChiSquareNoiseModel<>*>(params.m_NoiseModel.get()))
      m_NoiseModel = std::make_shared< mitk::ChiSquareNoiseModel<> >();
    m_NoiseModel->SetNoiseVariance(params.m_NoiseModel->GetNoiseVariance());
  }

  for (unsigned int i=0; i<params.m_FiberModelList.size()+params.m_NonFiberModelList.size(); i++)
  {
    mitk::DiffusionSignalModel<>* outModel = nullptr;
    mitk::DiffusionSignalModel<>* signalModel = nullptr;
    if (i<params.m_FiberModelList.size())
      signalModel = params.m_FiberModelList.at(i);
    else
      signalModel = params.m_NonFiberModelList.at(i-params.m_FiberModelList.size());

    if (dynamic_cast<mitk::StickModel<>*>(signalModel))
      outModel = new mitk::StickModel<>(dynamic_cast<mitk::StickModel<>*>(signalModel));
    else  if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
      outModel = new mitk::TensorModel<>(dynamic_cast<mitk::TensorModel<>*>(signalModel));
    else  if (dynamic_cast<mitk::RawShModel<>*>(signalModel))
      outModel = new mitk::RawShModel<>(dynamic_cast<mitk::RawShModel<>*>(signalModel));
    else  if (dynamic_cast<mitk::BallModel<>*>(signalModel))
      outModel = new mitk::BallModel<>(dynamic_cast<mitk::BallModel<>*>(signalModel));
    else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
      outModel = new mitk::AstroStickModel<>(dynamic_cast<mitk::AstroStickModel<>*>(signalModel));
    else  if (dynamic_cast<mitk::DotModel<>*>(signalModel))
      outModel = new mitk::DotModel<>(dynamic_cast<mitk::DotModel<>*>(signalModel));

    if (i<params.m_FiberModelList.size())
      m_FiberModelList.push_back(outModel);
    else
      m_NonFiberModelList.push_back(outModel);
  }
}


void mitk::FiberfoxParameters::ClearFiberParameters()
{
  m_Misc = MiscFiberfoxParameters();
  m_FiberGen = FiberGenerationParameters();
}

void mitk::FiberfoxParameters::ClearSignalParameters()
{
  m_Misc = MiscFiberfoxParameters();
  m_SignalGen = SignalGenerationParameters();

  m_FiberModelList.clear();
  m_NonFiberModelList.clear();
  m_NoiseModel = nullptr;
  m_MissingTags = "";
}

mitk::FiberfoxParameters::~FiberfoxParameters()
{

}

void mitk::SignalGenerationParameters::GenerateGradientHalfShell()
{
  int NPoints = 2*m_NumGradients;
  m_GradientDirections.clear();

  m_NumBaseline = NPoints/20;
  if (m_NumBaseline==0)
    m_NumBaseline=1;

  GradientType g;
  g.Fill(0.0);
  for (unsigned int i=0; i<m_NumBaseline; i++)
    m_GradientDirections.push_back(g);

  if (NPoints==0)
    return;

  vnl_vector<double> theta; theta.set_size(NPoints);
  vnl_vector<double> phi; phi.set_size(NPoints);
  double C = sqrt(4*itk::Math::pi);
  phi(0) = 0.0;
  phi(NPoints-1) = 0.0;
  for(int i=0; i<NPoints; i++)
  {
    theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - itk::Math::pi / 2.0;
    if( i>0 && i<NPoints-1)
    {
      phi(i) = (phi(i-1) + C /
                sqrt(NPoints*(1-(-1.0+2.0*i/(NPoints-1.0))*(-1.0+2.0*i/(NPoints-1.0)))));
    }
  }

  for(int i=0; i<NPoints; i++)
  {
    g[2] = sin(theta(i));
    if (g[2]<0)
      continue;
    g[0] = cos(theta(i)) * cos(phi(i));
    g[1] = cos(theta(i)) * sin(phi(i));
    m_GradientDirections.push_back(g);
  }
}

std::vector< int > mitk::SignalGenerationParameters::GetBaselineIndices()
{
  std::vector< int > result;
  for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
    if (m_GradientDirections.at(i).GetNorm()<0.0001)
      result.push_back(i);
  return result;
}

unsigned int mitk::SignalGenerationParameters::GetFirstBaselineIndex()
{
  for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
    if (m_GradientDirections.at(i).GetNorm()<0.0001)
      return i;
  return -1;
}

bool mitk::SignalGenerationParameters::IsBaselineIndex(unsigned int idx)
{
  if (m_GradientDirections.size()>idx && m_GradientDirections.at(idx).GetNorm()<0.0001)
    return true;
  return false;
}

unsigned int mitk::SignalGenerationParameters::GetNumWeightedVolumes()
{
  return m_NumGradients;
}

unsigned int mitk::SignalGenerationParameters::GetNumBaselineVolumes()
{
  return m_NumBaseline;
}

unsigned int mitk::SignalGenerationParameters::GetNumVolumes()
{
  return m_GradientDirections.size();
}

mitk::SignalGenerationParameters::GradientListType mitk::SignalGenerationParameters::GetGradientDirections()
{
  return m_GradientDirections;
}

mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer mitk::SignalGenerationParameters::GetItkGradientContainer()
{
  int c = 0;
  mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer out = mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::New();
  for (auto g : m_GradientDirections)
  {
    mitk::DiffusionPropertyHelper::GradientDirectionType vnl_dir;
    vnl_dir[0] = g[0];
    vnl_dir[1] = g[1];
    vnl_dir[2] = g[2];
    out->InsertElement(c, vnl_dir);
    ++c;
  }
  return out;
}

mitk::SignalGenerationParameters::GradientType mitk::SignalGenerationParameters::GetGradientDirection(unsigned int i)
{
  return m_GradientDirections.at(i);
}

void mitk::SignalGenerationParameters::SetNumWeightedVolumes(int numGradients)
{
  m_NumGradients = numGradients;
  GenerateGradientHalfShell();
}

std::vector< int > mitk::SignalGenerationParameters::GetBvalues()
{
  std::vector< int > bVals;
  for( GradientType g : m_GradientDirections)
  {
    float norm = g.GetNorm();
    int bVal = std::round(norm*norm*m_Bvalue);
    if ( std::find(bVals.begin(), bVals.end(), bVal) == bVals.end() )
      bVals.push_back(bVal);
  }
  return bVals;
}

double mitk::SignalGenerationParameters::GetBvalue()
{
  return m_Bvalue;
}

void mitk::SignalGenerationParameters::SetGradienDirections(GradientListType gradientList)
{
  m_GradientDirections = gradientList;
  m_NumGradients = 0;
  m_NumBaseline = 0;

  for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
  {
    float norm = m_GradientDirections.at(i).GetNorm();
    if (norm>0.0001)
      m_NumGradients++;
    else
      m_NumBaseline++;
  }
}

void mitk::SignalGenerationParameters::SetGradienDirections(mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientList)
{
  m_NumGradients = 0;
  m_NumBaseline = 0;
  m_GradientDirections.clear();

  for( unsigned int i=0; i<gradientList->Size(); i++)
  {
    GradientType g;
    g[0] = gradientList->at(i)[0];
    g[1] = gradientList->at(i)[1];
    g[2] = gradientList->at(i)[2];
    m_GradientDirections.push_back(g);

    float norm = m_GradientDirections.at(i).GetNorm();
    if (norm>0.0001)
      m_NumGradients++;
    else
      m_NumBaseline++;
  }
}

void mitk::SignalGenerationParameters::ApplyDirectionMatrix()
{
  auto imageRotationMatrix = m_ImageDirection.GetVnlMatrix();
  
  GradientListType rotated_gradients;
  for(auto g : m_GradientDirections)
  {
    vnl_vector<double> vec = g.GetVnlVector();
    vec = vec.pre_multiply(imageRotationMatrix);

    GradientType g2;
    g2[0] = vec[0];
    g2[1] = vec[1];
    g2[2] = vec[2];
    rotated_gradients.push_back(g2);
  }
  m_GradientDirections = rotated_gradients;
}

void mitk::FiberfoxParameters::ApplyDirectionMatrix()
{
  m_SignalGen.ApplyDirectionMatrix();
  UpdateSignalModels();
}

void mitk::FiberfoxParameters::SaveParameters(std::string filename)
{
  if(filename.empty())
    return;
  if(".ffp"!=filename.substr(filename.size()-4, 4))
    filename += ".ffp";

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

  boost::property_tree::ptree parameters;

  // fiber generation parameters
  parameters.put("fiberfox.fibers.distribution", m_FiberGen.m_Distribution);
  parameters.put("fiberfox.fibers.variance", m_FiberGen.m_Variance);
  parameters.put("fiberfox.fibers.density", m_FiberGen.m_Density);
  parameters.put("fiberfox.fibers.spline.sampling", m_FiberGen.m_Sampling);
  parameters.put("fiberfox.fibers.spline.tension", m_FiberGen.m_Tension);
  parameters.put("fiberfox.fibers.spline.continuity", m_FiberGen.m_Continuity);
  parameters.put("fiberfox.fibers.spline.bias", m_FiberGen.m_Bias);
  parameters.put("fiberfox.fibers.rotation.x", m_FiberGen.m_Rotation[0]);
  parameters.put("fiberfox.fibers.rotation.y", m_FiberGen.m_Rotation[1]);
  parameters.put("fiberfox.fibers.rotation.z", m_FiberGen.m_Rotation[2]);
  parameters.put("fiberfox.fibers.translation.x", m_FiberGen.m_Translation[0]);
  parameters.put("fiberfox.fibers.translation.y", m_FiberGen.m_Translation[1]);
  parameters.put("fiberfox.fibers.translation.z", m_FiberGen.m_Translation[2]);
  parameters.put("fiberfox.fibers.scale.x", m_FiberGen.m_Scale[0]);
  parameters.put("fiberfox.fibers.scale.y", m_FiberGen.m_Scale[1]);
  parameters.put("fiberfox.fibers.scale.z", m_FiberGen.m_Scale[2]);

  // image generation parameters
  parameters.put("fiberfox.image.basic.size.x", m_SignalGen.m_ImageRegion.GetSize(0));
  parameters.put("fiberfox.image.basic.size.y", m_SignalGen.m_ImageRegion.GetSize(1));
  parameters.put("fiberfox.image.basic.size.z", m_SignalGen.m_ImageRegion.GetSize(2));
  parameters.put("fiberfox.image.basic.spacing.x", m_SignalGen.m_ImageSpacing[0]);
  parameters.put("fiberfox.image.basic.spacing.y", m_SignalGen.m_ImageSpacing[1]);
  parameters.put("fiberfox.image.basic.spacing.z", m_SignalGen.m_ImageSpacing[2]);
  parameters.put("fiberfox.image.basic.origin.x", m_SignalGen.m_ImageOrigin[0]);
  parameters.put("fiberfox.image.basic.origin.y", m_SignalGen.m_ImageOrigin[1]);
  parameters.put("fiberfox.image.basic.origin.z", m_SignalGen.m_ImageOrigin[2]);
  parameters.put("fiberfox.image.basic.direction.d1", m_SignalGen.m_ImageDirection[0][0]);
  parameters.put("fiberfox.image.basic.direction.d2", m_SignalGen.m_ImageDirection[0][1]);
  parameters.put("fiberfox.image.basic.direction.d3", m_SignalGen.m_ImageDirection[0][2]);
  parameters.put("fiberfox.image.basic.direction.d4", m_SignalGen.m_ImageDirection[1][0]);
  parameters.put("fiberfox.image.basic.direction.d5", m_SignalGen.m_ImageDirection[1][1]);
  parameters.put("fiberfox.image.basic.direction.d6", m_SignalGen.m_ImageDirection[1][2]);
  parameters.put("fiberfox.image.basic.direction.d7", m_SignalGen.m_ImageDirection[2][0]);
  parameters.put("fiberfox.image.basic.direction.d8", m_SignalGen.m_ImageDirection[2][1]);
  parameters.put("fiberfox.image.basic.direction.d9", m_SignalGen.m_ImageDirection[2][2]);

  mitk::gradients::WriteBvalsBvecs(filename+".bvals", filename+".bvecs", m_SignalGen.GetItkGradientContainer(), m_SignalGen.m_Bvalue);

  parameters.put("fiberfox.image.acquisitiontype", m_SignalGen.m_AcquisitionType);
  parameters.put("fiberfox.image.coilsensitivityprofile", m_SignalGen.m_CoilSensitivityProfile);
  parameters.put("fiberfox.image.numberofcoils", m_SignalGen.m_NumberOfCoils);
  parameters.put("fiberfox.image.reversephase", m_SignalGen.m_ReversePhase);
  parameters.put("fiberfox.image.partialfourier", m_SignalGen.m_PartialFourier);
  parameters.put("fiberfox.image.noisevariance", m_SignalGen.m_NoiseVariance);
  parameters.put("fiberfox.image.trep", m_SignalGen.m_tRep);
  parameters.put("fiberfox.image.tinv", m_SignalGen.m_tInv);
  parameters.put("fiberfox.image.signalScale", m_SignalGen.m_SignalScale);
  parameters.put("fiberfox.image.tEcho", m_SignalGen.m_tEcho);
  parameters.put("fiberfox.image.tLine", m_SignalGen.m_tLine);
  parameters.put("fiberfox.image.tInhom", m_SignalGen.m_tInhom);
  parameters.put("fiberfox.image.echoTrainLength", m_SignalGen.m_EchoTrainLength);
  parameters.put("fiberfox.image.simulatekspace", m_SignalGen.m_SimulateKspaceAcquisition);
  parameters.put("fiberfox.image.axonRadius", m_SignalGen.m_AxonRadius);
  parameters.put("fiberfox.image.doSimulateRelaxation", m_SignalGen.m_DoSimulateRelaxation);
  parameters.put("fiberfox.image.doDisablePartialVolume", m_SignalGen.m_DoDisablePartialVolume);
  parameters.put("fiberfox.image.artifacts.spikesnum", m_SignalGen.m_Spikes);
  parameters.put("fiberfox.image.artifacts.spikesscale", m_SignalGen.m_SpikeAmplitude);
  parameters.put("fiberfox.image.artifacts.kspaceLineOffset", m_SignalGen.m_KspaceLineOffset);
  parameters.put("fiberfox.image.artifacts.eddyStrength", m_SignalGen.m_EddyStrength);
  parameters.put("fiberfox.image.artifacts.eddyTau", m_SignalGen.m_Tau);
  parameters.put("fiberfox.image.artifacts.aliasingfactor", m_SignalGen.m_CroppingFactor);
  parameters.put("fiberfox.image.artifacts.drift", m_SignalGen.m_Drift);

  parameters.put("fiberfox.image.artifacts.doAddMotion", m_SignalGen.m_DoAddMotion);
  parameters.put("fiberfox.image.artifacts.randomMotion", m_SignalGen.m_DoRandomizeMotion);
  parameters.put("fiberfox.image.artifacts.translation0", m_SignalGen.m_Translation[0]);
  parameters.put("fiberfox.image.artifacts.translation1", m_SignalGen.m_Translation[1]);
  parameters.put("fiberfox.image.artifacts.translation2", m_SignalGen.m_Translation[2]);
  parameters.put("fiberfox.image.artifacts.rotation0", m_SignalGen.m_Rotation[0]);
  parameters.put("fiberfox.image.artifacts.rotation1", m_SignalGen.m_Rotation[1]);
  parameters.put("fiberfox.image.artifacts.rotation2", m_SignalGen.m_Rotation[2]);
  parameters.put("fiberfox.image.artifacts.motionvolumes", m_Misc.m_MotionVolumesBox);

  parameters.put("fiberfox.image.artifacts.addringing", m_SignalGen.m_DoAddGibbsRinging);
  parameters.put("fiberfox.image.artifacts.zeroringing", m_SignalGen.m_ZeroRinging);
  parameters.put("fiberfox.image.artifacts.addnoise", m_Misc.m_DoAddNoise);
  parameters.put("fiberfox.image.artifacts.addghosts", m_Misc.m_DoAddGhosts);
  parameters.put("fiberfox.image.artifacts.addaliasing", m_Misc.m_DoAddAliasing);
  parameters.put("fiberfox.image.artifacts.addspikes", m_Misc.m_DoAddSpikes);
  parameters.put("fiberfox.image.artifacts.addeddycurrents", m_Misc.m_DoAddEddyCurrents);
  parameters.put("fiberfox.image.artifacts.doAddDistortions", m_Misc.m_DoAddDistortions);
  parameters.put("fiberfox.image.artifacts.doAddDrift", m_SignalGen.m_DoAddDrift);

  parameters.put("fiberfox.image.outputvolumefractions", m_Misc.m_OutputAdditionalImages);
  parameters.put("fiberfox.image.showadvanced", m_Misc.m_CheckAdvancedSignalOptionsBox);
  parameters.put("fiberfox.image.signalmodelstring", m_Misc.m_SignalModelString);
  parameters.put("fiberfox.image.artifactmodelstring", m_Misc.m_ArtifactModelString);
  parameters.put("fiberfox.image.outpath", m_Misc.m_OutputPath);
  parameters.put("fiberfox.fibers.realtime", m_Misc.m_CheckRealTimeFibersBox);
  parameters.put("fiberfox.fibers.showadvanced", m_Misc.m_CheckAdvancedFiberOptionsBox);
  parameters.put("fiberfox.fibers.constantradius", m_Misc.m_CheckConstantRadiusBox);
  parameters.put("fiberfox.fibers.includeFiducials", m_Misc.m_CheckIncludeFiducialsBox);

  if (m_NoiseModel!=nullptr)
  {
    parameters.put("fiberfox.image.artifacts.noisevariance", m_NoiseModel->GetNoiseVariance());
    if (dynamic_cast<mitk::RicianNoiseModel<>*>(m_NoiseModel.get()))
      parameters.put("fiberfox.image.artifacts.noisetype", "rice");
    else if (dynamic_cast<mitk::ChiSquareNoiseModel<>*>(m_NoiseModel.get()))
      parameters.put("fiberfox.image.artifacts.noisetype", "chisquare");
  }

  for (std::size_t i=0; i<m_FiberModelList.size()+m_NonFiberModelList.size(); i++)
  {
    mitk::DiffusionSignalModel<>* signalModel = nullptr;
    if (i<m_FiberModelList.size())
    {
      signalModel = m_FiberModelList.at(i);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".type", "fiber");
    }
    else
    {
      signalModel = m_NonFiberModelList.at(i-m_FiberModelList.size());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".type", "non-fiber");
    }

    if (dynamic_cast<mitk::StickModel<>*>(signalModel))
    {
      mitk::StickModel<>* model = dynamic_cast<mitk::StickModel<>*>(signalModel);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".model", "stick");
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".d", model->GetDiffusivity());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t2", model->GetT2());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t1", model->GetT1());
    }
    else  if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
    {
      mitk::TensorModel<>* model = dynamic_cast<mitk::TensorModel<>*>(signalModel);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".model", "tensor");
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".d1", model->GetDiffusivity1());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".d2", model->GetDiffusivity2());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".d3", model->GetDiffusivity3());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t2", model->GetT2());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t1", model->GetT1());
    }
    else  if (dynamic_cast<mitk::RawShModel<>*>(signalModel))
    {
      mitk::RawShModel<>* model = dynamic_cast<mitk::RawShModel<>*>(signalModel);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".model", "prototype");
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".minFA", model->GetFaRange().first);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".maxFA", model->GetFaRange().second);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".minADC", model->GetAdcRange().first);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".maxADC", model->GetAdcRange().second);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".maxNumSamples", model->GetMaxNumKernels());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".numSamples", model->GetNumberOfKernels());
      int shOrder = model->GetShOrder();
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".numCoeffs", (shOrder*shOrder + shOrder + 2)/2 + shOrder);

      for (unsigned int j=0; j<model->GetNumberOfKernels(); j++)
      {
        vnl_vector< double > coeffs = model->GetCoefficients(j);
        for (unsigned int k=0; k<coeffs.size(); k++)
          parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".kernels."+boost::lexical_cast<std::string>(j)+".coeffs."+boost::lexical_cast<std::string>(k), coeffs[k]);
        parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".kernels."+boost::lexical_cast<std::string>(j)+".B0", model->GetBaselineSignal(j));
      }
    }
    else  if (dynamic_cast<mitk::BallModel<>*>(signalModel))
    {
      mitk::BallModel<>* model = dynamic_cast<mitk::BallModel<>*>(signalModel);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".model", "ball");
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".d", model->GetDiffusivity());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t2", model->GetT2());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t1", model->GetT1());
    }
    else  if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
    {
      mitk::AstroStickModel<>* model = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".model", "astrosticks");
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".d", model->GetDiffusivity());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t2", model->GetT2());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t1", model->GetT1());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".randomize", model->GetRandomizeSticks());
    }
    else  if (dynamic_cast<mitk::DotModel<>*>(signalModel))
    {
      mitk::DotModel<>* model = dynamic_cast<mitk::DotModel<>*>(signalModel);
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".model", "dot");
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t2", model->GetT2());
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".t1", model->GetT1());
    }

    if (signalModel!=nullptr)
    {
      parameters.put("fiberfox.image.compartments.c"+boost::lexical_cast<std::string>(i)+".ID", signalModel->m_CompartmentId);

      if (signalModel->GetVolumeFractionImage().IsNotNull())
      {
        try{
          itk::ImageFileWriter<ItkDoubleImgType>::Pointer writer = itk::ImageFileWriter<ItkDoubleImgType>::New();
          writer->SetFileName(filename+"_VOLUME"+boost::lexical_cast<std::string>(signalModel->m_CompartmentId)+".nii.gz");
          writer->SetInput(signalModel->GetVolumeFractionImage());
          writer->Update();
          MITK_INFO << "Volume fraction image for compartment "+boost::lexical_cast<std::string>(signalModel->m_CompartmentId)+" saved.";
        }
        catch(...)
        {
        }
      }
    }
  }

  boost::property_tree::xml_writer_settings<std::string> writerSettings(' ', 2);
  boost::property_tree::xml_parser::write_xml(filename, parameters, std::locale(), writerSettings);

  try{
    itk::ImageFileWriter<ItkFloatImgType>::Pointer writer = itk::ImageFileWriter<ItkFloatImgType>::New();
    writer->SetFileName(filename+"_FMAP.nii.gz");
    writer->SetInput(m_SignalGen.m_FrequencyMap);
    writer->Update();
  }
  catch(...)
  {
    MITK_INFO << "No frequency map saved.";
  }
  try{
    itk::ImageFileWriter<ItkUcharImgType>::Pointer writer = itk::ImageFileWriter<ItkUcharImgType>::New();
    writer->SetFileName(filename+"_MASK.nii.gz");
    writer->SetInput(m_SignalGen.m_MaskImage);
    writer->Update();
  }
  catch(...)
  {
    MITK_INFO << "No mask image saved.";
  }

  setlocale(LC_ALL, currLocale.c_str());
}



template< class ParameterType >
ParameterType mitk::FiberfoxParameters::ReadVal(boost::property_tree::ptree::value_type const& v, std::string tag, ParameterType defaultValue, bool essential)
{
  try
  {
    return v.second.get<ParameterType>(tag);
  }
  catch (...)
  {
    if (essential)
    {
      mitkThrow() << "Parameter file corrupted. Essential tag is missing: '" << tag << "'";
    }
    if (tag!="artifacts.noisetype")
    {
      MITK_INFO << "Tag '" << tag << "' not found. Using default value '" << defaultValue << "'.";
      m_MissingTags += "\n- ";
      m_MissingTags += tag;
    }
    return defaultValue;
  }
}

void mitk::FiberfoxParameters::UpdateSignalModels()
{
  for (mitk::DiffusionSignalModel<>* m : m_FiberModelList)
  {
    m->SetGradientList(m_SignalGen.m_GradientDirections);
    m->SetBvalue(m_SignalGen.m_Bvalue);
  }
  for (mitk::DiffusionSignalModel<>* m : m_NonFiberModelList)
  {
    m->SetGradientList(m_SignalGen.m_GradientDirections);
    m->SetBvalue(m_SignalGen.m_Bvalue);
  }
}

void mitk::FiberfoxParameters::SetNumWeightedVolumes(int numGradients)
{
  m_SignalGen.SetNumWeightedVolumes(numGradients);
  UpdateSignalModels();
}

void mitk::FiberfoxParameters::SetGradienDirections(mitk::SignalGenerationParameters::GradientListType gradientList)
{
  m_SignalGen.SetGradienDirections(gradientList);
  UpdateSignalModels();
}

void mitk::FiberfoxParameters::SetGradienDirections(mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientList)
{
  m_SignalGen.SetGradienDirections(gradientList);
  UpdateSignalModels();
}

void mitk::FiberfoxParameters::SetBvalue(double Bvalue)
{
  m_SignalGen.m_Bvalue = Bvalue;
  UpdateSignalModels();
}

void mitk::FiberfoxParameters::GenerateGradientHalfShell()
{
  m_SignalGen.GenerateGradientHalfShell();
  UpdateSignalModels();
}

void mitk::FiberfoxParameters::LoadParameters(std::string filename, bool fix_seed)
{
  itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randgen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
  if (fix_seed)
  {
    srand(0);
    randgen->SetSeed(0);
  }
  else
  {
    srand(time(0));
    randgen->SetSeed();
  }
  m_MissingTags = "";
  if(filename.empty()) { return; }

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, nullptr );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }


  boost::property_tree::ptree parameterTree;
  boost::property_tree::xml_parser::read_xml( filename, parameterTree );


  m_FiberModelList.clear();
  m_NonFiberModelList.clear();
  if (m_NoiseModel) { m_NoiseModel = nullptr; }

  BOOST_FOREACH( boost::property_tree::ptree::value_type const& v1, parameterTree.get_child("fiberfox") )
  {
    if( v1.first == "fibers" )
    {
      m_Misc.m_CheckRealTimeFibersBox = ReadVal<bool>(v1,"realtime", m_Misc.m_CheckRealTimeFibersBox);
      m_Misc.m_CheckAdvancedFiberOptionsBox = ReadVal<bool>(v1,"showadvanced", m_Misc.m_CheckAdvancedFiberOptionsBox);
      m_Misc.m_CheckConstantRadiusBox = ReadVal<bool>(v1,"constantradius", m_Misc.m_CheckConstantRadiusBox);
      m_Misc.m_CheckIncludeFiducialsBox = ReadVal<bool>(v1,"includeFiducials", m_Misc.m_CheckIncludeFiducialsBox);

      switch (ReadVal<unsigned int>(v1,"distribution", 0))
      {
      case 0:
        m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
        break;
      case 1:
        m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_GAUSSIAN;
        break;
      default:
        m_FiberGen.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
      }
      m_FiberGen.m_Variance = ReadVal<double>(v1,"variance", m_FiberGen.m_Variance);
      m_FiberGen.m_Density = ReadVal<unsigned int>(v1,"density", m_FiberGen.m_Density);
      m_FiberGen.m_Sampling = ReadVal<double>(v1,"spline.sampling", m_FiberGen.m_Sampling);
      m_FiberGen.m_Tension = ReadVal<double>(v1,"spline.tension", m_FiberGen.m_Tension);
      m_FiberGen.m_Continuity = ReadVal<double>(v1,"spline.continuity", m_FiberGen.m_Continuity);
      m_FiberGen.m_Bias = ReadVal<double>(v1,"spline.bias", m_FiberGen.m_Bias);
      m_FiberGen.m_Rotation[0] = ReadVal<double>(v1,"rotation.x", m_FiberGen.m_Rotation[0]);
      m_FiberGen.m_Rotation[1] = ReadVal<double>(v1,"rotation.y", m_FiberGen.m_Rotation[1]);
      m_FiberGen.m_Rotation[2] = ReadVal<double>(v1,"rotation.z", m_FiberGen.m_Rotation[2]);
      m_FiberGen.m_Translation[0] = ReadVal<double>(v1,"translation.x", m_FiberGen.m_Translation[0]);
      m_FiberGen.m_Translation[1] = ReadVal<double>(v1,"translation.y", m_FiberGen.m_Translation[1]);
      m_FiberGen.m_Translation[2] = ReadVal<double>(v1,"translation.z", m_FiberGen.m_Translation[2]);
      m_FiberGen.m_Scale[0] = ReadVal<double>(v1,"scale.x", m_FiberGen.m_Scale[0]);
      m_FiberGen.m_Scale[1] = ReadVal<double>(v1,"scale.y", m_FiberGen.m_Scale[1]);
      m_FiberGen.m_Scale[2] = ReadVal<double>(v1,"scale.z", m_FiberGen.m_Scale[2]);
    }
    else if ( v1.first == "image" )
    {
      m_Misc.m_SignalModelString = ReadVal<std::string>(v1,"signalmodelstring", m_Misc.m_SignalModelString);
      m_Misc.m_ArtifactModelString = ReadVal<std::string>(v1,"artifactmodelstring", m_Misc.m_ArtifactModelString);
      m_Misc.m_OutputPath = ReadVal<std::string>(v1,"outpath", m_Misc.m_OutputPath);
      m_Misc.m_OutputAdditionalImages = ReadVal<bool>(v1,"outputvolumefractions", m_Misc.m_OutputAdditionalImages);
      m_Misc.m_CheckAdvancedSignalOptionsBox = ReadVal<bool>(v1,"showadvanced", m_Misc.m_CheckAdvancedSignalOptionsBox);
      m_Misc.m_DoAddDistortions = ReadVal<bool>(v1,"artifacts.doAddDistortions", m_Misc.m_DoAddDistortions);
      m_Misc.m_DoAddNoise = ReadVal<bool>(v1,"artifacts.addnoise", m_Misc.m_DoAddNoise);
      m_Misc.m_DoAddGhosts = ReadVal<bool>(v1,"artifacts.addghosts", m_Misc.m_DoAddGhosts);
      m_Misc.m_DoAddAliasing = ReadVal<bool>(v1,"artifacts.addaliasing", m_Misc.m_DoAddAliasing);
      m_Misc.m_DoAddSpikes = ReadVal<bool>(v1,"artifacts.addspikes", m_Misc.m_DoAddSpikes);
      m_Misc.m_DoAddEddyCurrents = ReadVal<bool>(v1,"artifacts.addeddycurrents", m_Misc.m_DoAddEddyCurrents);

      m_SignalGen.m_ImageRegion.SetSize(0, ReadVal<int>(v1,"basic.size.x",m_SignalGen.m_ImageRegion.GetSize(0)));
      m_SignalGen.m_ImageRegion.SetSize(1, ReadVal<int>(v1,"basic.size.y",m_SignalGen.m_ImageRegion.GetSize(1)));
      m_SignalGen.m_ImageRegion.SetSize(2, ReadVal<int>(v1,"basic.size.z",m_SignalGen.m_ImageRegion.GetSize(2)));
      m_SignalGen.m_ImageSpacing[0] = ReadVal<float>(v1,"basic.spacing.x",m_SignalGen.m_ImageSpacing[0]);
      m_SignalGen.m_ImageSpacing[1] = ReadVal<float>(v1,"basic.spacing.y",m_SignalGen.m_ImageSpacing[1]);
      m_SignalGen.m_ImageSpacing[2] = ReadVal<float>(v1,"basic.spacing.z",m_SignalGen.m_ImageSpacing[2]);
      m_SignalGen.m_ImageOrigin[0] = ReadVal<float>(v1,"basic.origin.x",m_SignalGen.m_ImageOrigin[0]);
      m_SignalGen.m_ImageOrigin[1] = ReadVal<float>(v1,"basic.origin.y",m_SignalGen.m_ImageOrigin[1]);
      m_SignalGen.m_ImageOrigin[2] = ReadVal<float>(v1,"basic.origin.z",m_SignalGen.m_ImageOrigin[2]);

      int i = 0;
      int j = 0;
      for(auto v : v1.second.get_child("basic.direction"))
      {
        m_SignalGen.m_ImageDirection[i][j] = boost::lexical_cast<float>(v.second.data());
        ++j;
        if (j==3)
        {
          j = 0;
          ++i;
        }
      }

      m_SignalGen.m_AcquisitionType = (SignalGenerationParameters::AcquisitionType)ReadVal<int>(v1,"acquisitiontype", m_SignalGen.m_AcquisitionType);
      m_SignalGen.m_CoilSensitivityProfile = (SignalGenerationParameters::CoilSensitivityProfile)ReadVal<int>(v1,"coilsensitivityprofile", m_SignalGen.m_CoilSensitivityProfile);
      m_SignalGen.m_NumberOfCoils = ReadVal<unsigned int>(v1,"numberofcoils", m_SignalGen.m_NumberOfCoils);
      m_SignalGen.m_ReversePhase = ReadVal<bool>(v1,"reversephase", m_SignalGen.m_ReversePhase);
      m_SignalGen.m_PartialFourier = ReadVal<float>(v1,"partialfourier", m_SignalGen.m_PartialFourier);
      m_SignalGen.m_NoiseVariance = ReadVal<float>(v1,"noisevariance", m_SignalGen.m_NoiseVariance);
      m_SignalGen.m_tRep = ReadVal<float>(v1,"trep", m_SignalGen.m_tRep);
      m_SignalGen.m_tInv = ReadVal<float>(v1,"tinv", m_SignalGen.m_tInv);
      m_SignalGen.m_SignalScale = ReadVal<float>(v1,"signalScale", m_SignalGen.m_SignalScale);
      m_SignalGen.m_tEcho = ReadVal<float>(v1,"tEcho", m_SignalGen.m_tEcho);
      m_SignalGen.m_tLine = ReadVal<float>(v1,"tLine", m_SignalGen.m_tLine);
      m_SignalGen.m_tInhom = ReadVal<float>(v1,"tInhom", m_SignalGen.m_tInhom);
      m_SignalGen.m_EchoTrainLength = ReadVal<unsigned int>(v1,"echoTrainLength", m_SignalGen.m_EchoTrainLength);
      m_SignalGen.m_SimulateKspaceAcquisition = ReadVal<bool>(v1,"simulatekspace", m_SignalGen.m_SimulateKspaceAcquisition);

      m_SignalGen.m_AxonRadius = ReadVal<double>(v1,"axonRadius", m_SignalGen.m_AxonRadius);
      m_SignalGen.m_Spikes = ReadVal<unsigned int>(v1,"artifacts.spikesnum", m_SignalGen.m_Spikes);
      m_SignalGen.m_SpikeAmplitude = ReadVal<float>(v1,"artifacts.spikesscale", m_SignalGen.m_SpikeAmplitude);
      m_SignalGen.m_KspaceLineOffset = ReadVal<float>(v1,"artifacts.kspaceLineOffset", m_SignalGen.m_KspaceLineOffset);
      m_SignalGen.m_EddyStrength = ReadVal<float>(v1,"artifacts.eddyStrength", m_SignalGen.m_EddyStrength);
      m_SignalGen.m_Tau = ReadVal<float>(v1,"artifacts.eddyTau", m_SignalGen.m_Tau);
      m_SignalGen.m_CroppingFactor = ReadVal<float>(v1,"artifacts.aliasingfactor", m_SignalGen.m_CroppingFactor);
      m_SignalGen.m_Drift = ReadVal<float>(v1,"artifacts.drift", m_SignalGen.m_Drift);
      m_SignalGen.m_DoAddGibbsRinging = ReadVal<bool>(v1,"artifacts.addringing", m_SignalGen.m_DoAddGibbsRinging);
      m_SignalGen.m_ZeroRinging = ReadVal<int>(v1,"artifacts.zeroringing", m_SignalGen.m_ZeroRinging);
      m_SignalGen.m_DoSimulateRelaxation = ReadVal<bool>(v1,"doSimulateRelaxation", m_SignalGen.m_DoSimulateRelaxation);
      m_SignalGen.m_DoDisablePartialVolume = ReadVal<bool>(v1,"doDisablePartialVolume", m_SignalGen.m_DoDisablePartialVolume);
      m_SignalGen.m_DoAddMotion = ReadVal<bool>(v1,"artifacts.doAddMotion", m_SignalGen.m_DoAddMotion);
      m_SignalGen.m_DoRandomizeMotion = ReadVal<bool>(v1,"artifacts.randomMotion", m_SignalGen.m_DoRandomizeMotion);
      m_SignalGen.m_DoAddDrift = ReadVal<bool>(v1,"artifacts.doAddDrift", m_SignalGen.m_DoAddDrift);
      m_SignalGen.m_Translation[0] = ReadVal<float>(v1,"artifacts.translation0", m_SignalGen.m_Translation[0]);
      m_SignalGen.m_Translation[1] = ReadVal<float>(v1,"artifacts.translation1", m_SignalGen.m_Translation[1]);
      m_SignalGen.m_Translation[2] = ReadVal<float>(v1,"artifacts.translation2", m_SignalGen.m_Translation[2]);
      m_SignalGen.m_Rotation[0] = ReadVal<float>(v1,"artifacts.rotation0", m_SignalGen.m_Rotation[0]);
      m_SignalGen.m_Rotation[1] = ReadVal<float>(v1,"artifacts.rotation1", m_SignalGen.m_Rotation[1]);
      m_SignalGen.m_Rotation[2] = ReadVal<float>(v1,"artifacts.rotation2", m_SignalGen.m_Rotation[2]);

      if (itksys::SystemTools::FileExists(filename+".bvals") && itksys::SystemTools::FileExists(filename+".bvecs"))
      {
        m_Misc.m_BvalsFile = filename+".bvals";
        m_Misc.m_BvecsFile = filename+".bvecs";
        m_SignalGen.SetGradienDirections( mitk::gradients::ReadBvalsBvecs(m_Misc.m_BvalsFile, m_Misc.m_BvecsFile, m_SignalGen.m_Bvalue) );
      }
      else
      {
        m_SignalGen.m_Bvalue = ReadVal<double>(v1,"bvalue", m_SignalGen.m_Bvalue);
        SignalGenerationParameters::GradientListType gradients;
        try
        {
          BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second.get_child("gradients") )
          {
            SignalGenerationParameters::GradientType g;
            g[0] = ReadVal<double>(v2,"x",0);
            g[1] = ReadVal<double>(v2,"y",0);
            g[2] = ReadVal<double>(v2,"z",0);
            gradients.push_back(g);
          }
        }
        catch(...)
        {
          MITK_INFO << "WARNING: Fiberfox parameters without any gradient directions loaded.";
        }
        m_SignalGen.SetGradienDirections(gradients);
      }

      m_Misc.m_MotionVolumesBox = ReadVal<std::string>(v1,"artifacts.motionvolumes", m_Misc.m_MotionVolumesBox);
      m_SignalGen.m_MotionVolumes.clear();

      if ( m_Misc.m_MotionVolumesBox == "random" )
      {
        m_SignalGen.m_MotionVolumes.push_back(0);
        for ( size_t i=1; i < m_SignalGen.GetNumVolumes(); ++i )
        {
          m_SignalGen.m_MotionVolumes.push_back( bool( randgen->GetIntegerVariate()%2 ) );
        }
        MITK_DEBUG << "mitkFiberfoxParameters.cpp: Case m_Misc.m_MotionVolumesBox == \"random\".";
      }
      else if ( ! m_Misc.m_MotionVolumesBox.empty() )
      {
        std::stringstream stream( m_Misc.m_MotionVolumesBox );
        std::vector<int> numbers;
        int nummer = std::numeric_limits<int>::max();
        while( stream >> nummer )
        {
          if( nummer < std::numeric_limits<int>::max() )
          {
            numbers.push_back( nummer );
          }
        }
        // If a list of negative numbers is given:
        if( *(std::min_element( numbers.begin(), numbers.end() )) < 0
            && *(std::max_element( numbers.begin(), numbers.end() )) <= 0 ) // cave: -0 == +0
        {
          for ( size_t i=0; i<m_SignalGen.GetNumVolumes(); ++i )
          {
            m_SignalGen.m_MotionVolumes.push_back( true );
          }
          // set all true except those given.
          for (auto number : numbers)
          {
            if (-number < static_cast<int>(m_SignalGen.GetNumVolumes()) && -number >= 0 )
              m_SignalGen.m_MotionVolumes.at(-number) = false;
          }
          MITK_DEBUG << "mitkFiberfoxParameters.cpp: Case list of negative numbers.";
        }
        // If a list of positive numbers is given:
        else if( *(std::min_element( numbers.begin(), numbers.end() )) >= 0
                 && *(std::max_element( numbers.begin(), numbers.end() )) >= 0 )
        {
          for ( size_t i=0; i<m_SignalGen.GetNumVolumes(); ++i )
          {
            m_SignalGen.m_MotionVolumes.push_back( false );
          }
          // set all false except those given.
          for (auto number : numbers)
          {
            if (number < static_cast<int>(m_SignalGen.GetNumVolumes()) && number >= 0)
              m_SignalGen.m_MotionVolumes.at(number) = true;
          }
          MITK_DEBUG << "mitkFiberfoxParameters.cpp: Case list of positive numbers.";
        }
        else
        {
          MITK_WARN << "mitkFiberfoxParameters.cpp: Inconsistent list of numbers in m_MotionVolumesBox.";
          break;
        }
      }
      else
      {
        MITK_WARN << "mitkFiberfoxParameters.cpp: Cannot make sense of string in m_MotionVolumesBox.";
        break;
      }


      try
      {
        if (ReadVal<std::string>(v1,"artifacts.noisetype","")=="rice")
        {
          m_NoiseModel = std::make_shared< mitk::RicianNoiseModel<> >();
          m_NoiseModel->SetNoiseVariance(ReadVal<float>(v1,"artifacts.noisevariance",m_NoiseModel->GetNoiseVariance()));
        }
      }
      catch(...)
      {
        MITK_DEBUG << "mitkFiberfoxParameters.cpp: caught some error while trying m_NoiseModel->SetNoiseVariance()";
        // throw;
      }

      try
      {
        if (ReadVal<std::string>(v1,"artifacts.noisetype","")=="chisquare")
        {
          m_NoiseModel = std::make_shared< mitk::ChiSquareNoiseModel<> >();
          m_NoiseModel->SetNoiseVariance(ReadVal<float>(v1,"artifacts.noisevariance",m_NoiseModel->GetNoiseVariance()));
        }
      }
      catch(...)
      {
        MITK_DEBUG << "mitkFiberfoxParameters.cpp: caught some error while trying m_NoiseModel->SetNoiseVariance()";
        // throw;
      }


      BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second.get_child("compartments") )
      {
        mitk::DiffusionSignalModel<>* signalModel = nullptr;

        std::string model = ReadVal<std::string>(v2,"model","",true);
        if (model=="stick")
        {
          mitk::StickModel<>* model = new mitk::StickModel<>();
          model->SetDiffusivity(ReadVal<double>(v2,"d",model->GetDiffusivity()));
          model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
          model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
          model->SetBvalue(m_SignalGen.m_Bvalue);
          model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
          if (ReadVal<std::string>(v2,"type","",true)=="fiber")
            m_FiberModelList.push_back(model);
          else if (ReadVal<std::string>(v2,"type","",true)=="non-fiber")
            m_NonFiberModelList.push_back(model);
          signalModel = model;
        }
        else if (model=="tensor")
        {
          mitk::TensorModel<>* model = new mitk::TensorModel<>();
          model->SetDiffusivity1(ReadVal<double>(v2,"d1",model->GetDiffusivity1()));
          model->SetDiffusivity2(ReadVal<double>(v2,"d2",model->GetDiffusivity2()));
          model->SetDiffusivity3(ReadVal<double>(v2,"d3",model->GetDiffusivity3()));
          model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
          model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
          model->SetBvalue(m_SignalGen.m_Bvalue);
          model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
          if (ReadVal<std::string>(v2,"type","",true)=="fiber")
            m_FiberModelList.push_back(model);
          else if (ReadVal<std::string>(v2,"type","",true)=="non-fiber")
            m_NonFiberModelList.push_back(model);
          signalModel = model;
        }
        else if (model=="ball")
        {
          mitk::BallModel<>* model = new mitk::BallModel<>();
          model->SetDiffusivity(ReadVal<double>(v2,"d",model->GetDiffusivity()));
          model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
          model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
          model->SetBvalue(m_SignalGen.m_Bvalue);
          model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
          if (ReadVal<std::string>(v2,"type","",true)=="fiber")
            m_FiberModelList.push_back(model);
          else if (ReadVal<std::string>(v2,"type","",true)=="non-fiber")
            m_NonFiberModelList.push_back(model);
          signalModel = model;
        }
        else if (model=="astrosticks")
        {
          mitk::AstroStickModel<>* model = new AstroStickModel<>();
          model->SetDiffusivity(ReadVal<double>(v2,"d",model->GetDiffusivity()));
          model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
          model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
          model->SetBvalue(m_SignalGen.m_Bvalue);
          model->SetRandomizeSticks(ReadVal<bool>(v2,"randomize",model->GetRandomizeSticks()));
          model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
          if (ReadVal<std::string>(v2,"type","",true)=="fiber")
            m_FiberModelList.push_back(model);
          else if (ReadVal<std::string>(v2,"type","",true)=="non-fiber")
            m_NonFiberModelList.push_back(model);
          signalModel = model;
        }
        else if (model=="dot")
        {
          mitk::DotModel<>* model = new mitk::DotModel<>();
          model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
          model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
          model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
          if (ReadVal<std::string>(v2,"type","",true)=="fiber")
            m_FiberModelList.push_back(model);
          else if (ReadVal<std::string>(v2,"type","",true)=="non-fiber")
            m_NonFiberModelList.push_back(model);
          signalModel = model;
        }
        else if (model=="prototype")
        {
          mitk::RawShModel<>* model = new mitk::RawShModel<>();
          model->SetMaxNumKernels(ReadVal<unsigned int>(v2,"maxNumSamples",model->GetMaxNumKernels()));
          model->SetFaRange(ReadVal<double>(v2,"minFA",model->GetFaRange().first), ReadVal<double>(v2,"maxFA",model->GetFaRange().second));
          model->SetAdcRange(ReadVal<double>(v2,"minADC",model->GetAdcRange().first), ReadVal<double>(v2,"maxADC",model->GetAdcRange().second));
          model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
          unsigned int numCoeffs = ReadVal<unsigned int>(v2,"numCoeffs",0,true);
          unsigned int numSamples = ReadVal<unsigned int>(v2,"numSamples",0,true);

          for (unsigned int j=0; j<numSamples; j++)
          {
            vnl_vector< double > coeffs(numCoeffs);
            for (unsigned int k=0; k<numCoeffs; k++)
            {
              coeffs[k] = ReadVal<double>(v2,"kernels."+boost::lexical_cast<std::string>(j)+".coeffs."+boost::lexical_cast<std::string>(k),0,true);
            }
            model->SetShCoefficients( coeffs, ReadVal<double>(v2,"kernels."+boost::lexical_cast<std::string>(j)+".B0",0,true) );
          }

          if (ReadVal<std::string>(v2,"type","",true)=="fiber")
          { m_FiberModelList.push_back(model); }
          else if (ReadVal<std::string>(v2,"type","",true)=="non-fiber")
          { m_NonFiberModelList.push_back(model); }
          // else ?
          signalModel = model;
        }

        if (signalModel!=nullptr)
        {
          try
          {
            itk::ImageFileReader<ItkDoubleImgType>::Pointer reader = itk::ImageFileReader<ItkDoubleImgType>::New();
            if ( itksys::SystemTools::FileExists(filename+"_VOLUME"+ReadVal<std::string>(v2,"ID","")+".nii.gz") )
              reader->SetFileName(filename+"_VOLUME"+ReadVal<std::string>(v2,"ID","")+".nii.gz");
            else if ( itksys::SystemTools::FileExists(filename+"_VOLUME"+ReadVal<std::string>(v2,"ID","")+".nii") )
              reader->SetFileName(filename+"_VOLUME"+ReadVal<std::string>(v2,"ID","")+".nii");
            else
              reader->SetFileName(filename+"_VOLUME"+ReadVal<std::string>(v2,"ID","")+".nrrd");
            reader->Update();
            signalModel->SetVolumeFractionImage(reader->GetOutput());
            MITK_INFO << "Volume fraction image loaded for compartment " << signalModel->m_CompartmentId;
          }
          catch(...)
          {
            MITK_INFO << "No volume fraction image found for compartment " << signalModel->m_CompartmentId;
          }
        }
      }
    }
    else
    {

    }
  }

  UpdateSignalModels();

  try
  {
    itk::ImageFileReader<ItkFloatImgType>::Pointer reader = itk::ImageFileReader<ItkFloatImgType>::New();
    reader->SetFileName(filename+"_FMAP.nrrd");
    if ( itksys::SystemTools::FileExists(filename+"_FMAP.nii.gz") )
      reader->SetFileName(filename+"_FMAP.nii.gz");
    else if ( itksys::SystemTools::FileExists(filename+"_FMAP.nii") )
      reader->SetFileName(filename+"_FMAP.nii");
    else
      reader->SetFileName(filename+"_FMAP.nrrd");
    reader->Update();
    m_SignalGen.m_FrequencyMap = reader->GetOutput();
    MITK_INFO << "Frequency map loaded.";
  }
  catch(...)
  {
    MITK_INFO << "No frequency map found.";
  }

  try
  {
    itk::ImageFileReader<ItkUcharImgType>::Pointer reader = itk::ImageFileReader<ItkUcharImgType>::New();
    if ( itksys::SystemTools::FileExists(filename+"_MASK.nii.gz") )
      reader->SetFileName(filename+"_MASK.nii.gz");
    else if ( itksys::SystemTools::FileExists(filename+"_MASK.nii") )
      reader->SetFileName(filename+"_MASK.nii");
    else
      reader->SetFileName(filename+"_MASK.nrrd");
    reader->Update();
    m_SignalGen.m_MaskImage = reader->GetOutput();
    m_SignalGen.m_ImageRegion = m_SignalGen.m_MaskImage->GetLargestPossibleRegion();
    m_SignalGen.m_ImageSpacing = m_SignalGen.m_MaskImage->GetSpacing();
    m_SignalGen.m_ImageOrigin = m_SignalGen.m_MaskImage->GetOrigin();
    m_SignalGen.m_ImageDirection = m_SignalGen.m_MaskImage->GetDirection();
    MITK_INFO << "Mask image loaded.";
  }
  catch(...)
  {
    MITK_INFO << "No mask image found.";
  }

  setlocale(LC_ALL, currLocale.c_str());
}


void mitk::FiberfoxParameters::PrintSelf()
{
  MITK_INFO << "Not implemented :(";
}
