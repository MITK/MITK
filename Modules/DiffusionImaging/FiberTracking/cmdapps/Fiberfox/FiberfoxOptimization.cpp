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

#include <random>

#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkProperties.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>
#include "mitkCommandLineParser.h"

#include <itkTractsToDWIImageFilter.h>
#include <mitkLexicalCast.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itksys/SystemTools.hxx>
#include <mitkFiberfoxParameters.h>
#include <random>
#include <mitkTensorImage.h>
#include <itkTensorDerivedMeasurementsFilter.h>
#include <itkAdcImageFilter.h>
#include <itkMaskedImageToHistogramFilter.h>
#include <itkTdiToVolumeFractionFilter.h>
#include <itkImageFileReader.h>

using namespace mitk;

double CalcErrorSignal(const std::vector<double>& histo_mod, itk::VectorImage< short, 3 >* reference, itk::VectorImage< short, 3 >* simulation, itk::Image< unsigned char,3 >::Pointer mask, itk::Image< double,3 >::Pointer fa)
{
  typedef itk::Image< double, 3 > DoubleImageType;
  typedef itk::VectorImage< short, 3 > DwiImageType;
  if (fa.IsNotNull())
  {
    itk::ImageRegionIterator< DwiImageType > it1(reference, reference->GetLargestPossibleRegion());
    itk::ImageRegionIterator< DwiImageType > it2(simulation, simulation->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator< DoubleImageType > it3(fa, fa->GetLargestPossibleRegion());

    unsigned int count = 0;
    double error = 0;
    while(!it1.IsAtEnd())
    {
      if (mask.IsNull() || (mask.IsNotNull() && mask->GetLargestPossibleRegion().IsInside(it1.GetIndex()) && mask->GetPixel(it1.GetIndex())>0) )
      {
        double fa = it3.Get();
        if (fa>0)
        {
          double mod = 1.0;
          for (int i=histo_mod.size()-1; i>=0; --i)
            if (fa >= (double)i/histo_mod.size())
            {
              mod = histo_mod.at(i);
              break;
            }

          for (unsigned int i=0; i<reference->GetVectorLength(); ++i)
          {
            if (it1.Get()[i]>0)
            {
              double diff = (double)it2.Get()[i]/it1.Get()[i] - 1.0;
              error += std::pow(mod, 4) * fabs(diff);
              count++;
            }
          }
        }
      }
      ++it1;
      ++it2;
      ++it3;
    }
    return error/count;
  }
  else
  {
    itk::ImageRegionIterator< DwiImageType > it1(reference, reference->GetLargestPossibleRegion());
    itk::ImageRegionIterator< DwiImageType > it2(simulation, simulation->GetLargestPossibleRegion());

    unsigned int count = 0;
    double error = 0;
    while(!it1.IsAtEnd())
    {
      if (mask.IsNull() || (mask.IsNotNull() && mask->GetLargestPossibleRegion().IsInside(it1.GetIndex()) && mask->GetPixel(it1.GetIndex())>0) )
      {
        for (unsigned int i=0; i<reference->GetVectorLength(); ++i)
        {
          if (it1.Get()[i]>0)
          {
            double diff = (double)it2.Get()[i]/it1.Get()[i] - 1.0;
            error += fabs(diff);
            count++;
          }
        }
      }
      ++it1;
      ++it2;
    }
    return error/count;
  }
  return -1;
}

double CalcErrorFA(const std::vector<double>& histo_mod, mitk::Image::Pointer dwi1, itk::VectorImage< short, 3 >* dwi2, itk::Image< unsigned char,3 >::Pointer mask, itk::Image< double,3 >::Pointer fa1, itk::Image< double,3 >::Pointer md1, bool b0_contrast)
{
  typedef itk::TensorDerivedMeasurementsFilter<double> MeasurementsType;
  typedef itk::Image< double, 3 > DoubleImageType;
  typedef  itk::VectorImage< short, 3 > DwiType;

  DwiType::Pointer dwi1_itk = mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi1);
  typedef itk::DiffusionTensor3DReconstructionImageFilter<short, short, double > TensorReconstructionImageFilterType;
  DoubleImageType::Pointer fa2;
  {
    mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer gradientContainerCopy = mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::New();
    for(auto it = mitk::DiffusionPropertyHelper::GetGradientContainer(dwi1)->Begin(); it != mitk::DiffusionPropertyHelper::GetGradientContainer(dwi1)->End(); it++)
      gradientContainerCopy->push_back(it.Value());

    TensorReconstructionImageFilterType::Pointer tensorReconstructionFilter = TensorReconstructionImageFilterType::New();
    tensorReconstructionFilter->SetBValue( mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi1) );
    tensorReconstructionFilter->SetGradientImage(gradientContainerCopy, dwi2 );
    tensorReconstructionFilter->Update();

    MeasurementsType::Pointer measurementsCalculator = MeasurementsType::New();
    measurementsCalculator->SetInput( tensorReconstructionFilter->GetOutput() );
    measurementsCalculator->SetMeasure(MeasurementsType::FA);
    measurementsCalculator->Update();
    fa2 = measurementsCalculator->GetOutput();
  }

  DoubleImageType::Pointer md2;
  if (md1.IsNotNull())
  {
    typedef itk::AdcImageFilter< short, double > AdcFilterType;
    AdcFilterType::Pointer filter = AdcFilterType::New();
    filter->SetInput( dwi2 );
    filter->SetGradientDirections( mitk::DiffusionPropertyHelper::GetGradientContainer(dwi1) );
    filter->SetB_value( mitk::DiffusionPropertyHelper::GetReferenceBValue(dwi1) );
    filter->SetFitSignal(false);
    filter->Update();
    md2 = filter->GetOutput();
  }

  itk::ImageRegionConstIterator< DoubleImageType > it1(fa1, fa1->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator< DoubleImageType > it2(fa2, fa2->GetLargestPossibleRegion());

  itk::ImageRegionConstIterator< itk::VectorImage< short, 3 > > it_diff1(dwi1_itk, dwi1_itk->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator< itk::VectorImage< short, 3 > > it_diff2(dwi2, dwi2->GetLargestPossibleRegion());

  unsigned int count = 0;
  double error = 0;
  if (md1.IsNotNull() && md2.IsNotNull())
  {
    itk::ImageRegionConstIterator< DoubleImageType > it3(md1, md1->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator< DoubleImageType > it4(md2, md2->GetLargestPossibleRegion());
    while(!it1.IsAtEnd())
    {
      if (mask.IsNull() || (mask.IsNotNull() && mask->GetLargestPossibleRegion().IsInside(it1.GetIndex()) && mask->GetPixel(it1.GetIndex())>0) )
      {
        double fa = it1.Get();
        if (fa>0 && it3.Get()>0)
        {
          double mod = 1.0;
          for (int i=histo_mod.size()-1; i>=0; --i)
            if (fa >= (double)i/histo_mod.size())
            {
              mod = histo_mod.at(i);
              break;
            }

          double fa_diff = std::fabs(it2.Get()/fa - 1.0);
          double md_diff = std::fabs(it4.Get()/it3.Get() - 1.0);
          error += std::pow(mod, 4) * (fa_diff + md_diff);
          count += 2;

          if (b0_contrast && it_diff1.Get()[0]>0)
          {
            double b0_diff = (double)it_diff2.Get()[0]/it_diff1.Get()[0] - 1.0;
            error += std::pow(mod, 4) * std::fabs(b0_diff);
            ++count;
          }
        }
      }
      ++it1;
      ++it2;
      ++it3;
      ++it4;
      ++it_diff1;
      ++it_diff2;
    }
  }
  else
  {
    unsigned int count = 0;
    double error = 0;
    while(!it1.IsAtEnd())
    {
      if (mask.IsNull() || (mask.IsNotNull() && mask->GetLargestPossibleRegion().IsInside(it1.GetIndex()) && mask->GetPixel(it1.GetIndex())>0) )
      {
        double fa = it1.Get();
        if (fa>0)
        {
          double mod = 1.0;
          for (int i=histo_mod.size()-1; i>=0; --i)
            if (fa >= (double)i/histo_mod.size())
            {
              mod = histo_mod.at(i);
              break;
            }

          double fa_diff = fabs(it2.Get()/fa - 1.0);
          error += std::pow(mod, 4) * fa_diff;
          ++count;

          if (b0_contrast && it_diff1.Get()[0]>0)
          {
            double b0_diff = (double)it_diff2.Get()[0]/it_diff1.Get()[0] - 1.0;
            error += std::pow(mod, 4) * std::fabs(b0_diff);
            ++count;
          }
        }
      }
      ++it1;
      ++it2;
      ++it_diff1;
      ++it_diff2;
    }
  }

  return error/count;
}

FiberfoxParameters MakeProposalScale(FiberfoxParameters old_params, double temperature)
{
  FiberfoxParameters new_params(old_params);
  std::random_device r;
  std::default_random_engine randgen(r());

  std::normal_distribution<double> normal_dist(0, new_params.m_SignalGen.m_SignalScale*0.1*temperature);
  double add = 0;
  while (add == 0)
    add =  normal_dist(randgen);
  new_params.m_SignalGen.m_SignalScale += add;
  MITK_INFO << "Proposal Signal Scale: " << new_params.m_SignalGen.m_SignalScale << " (" << add << ")";
  return new_params;
}

FiberfoxParameters MakeProposalRelaxation(FiberfoxParameters old_params, double temperature)
{
  FiberfoxParameters new_params(old_params);
  std::random_device r;
  std::default_random_engine randgen(r());
  std::uniform_int_distribution<int> uint1(0, 3);
  int prop = uint1(randgen);

  switch(prop)
  {
  case 0:
  {
    int model_index = rand()%new_params.m_NonFiberModelList.size();
    double t2 = new_params.m_NonFiberModelList[model_index]->GetT2();
    std::normal_distribution<double> normal_dist(0, t2*0.1*temperature);

    double add = 0;
    while (add == 0)
      add = normal_dist(randgen);

    if ( (t2+add)*1.5 > new_params.m_NonFiberModelList[model_index]->GetT1() )
      add = -add;
    t2 += add;
    new_params.m_NonFiberModelList[model_index]->SetT2(t2);
    MITK_INFO << "Proposal T2 (Non-Fiber " << model_index << "): " << t2 << " (" << add << ")";
    break;
  }
  case 1:
  {
    int model_index = rand()%new_params.m_FiberModelList.size();
    double t2 = new_params.m_FiberModelList[model_index]->GetT2();
    std::normal_distribution<double> normal_dist(0, t2*0.1*temperature);

    double add = 0;
    while (add == 0)
      add = normal_dist(randgen);

    if ( (t2+add)*1.5 > new_params.m_FiberModelList[model_index]->GetT1() )
      add = -add;
    t2 += add;
    new_params.m_FiberModelList[model_index]->SetT2(t2);
    MITK_INFO << "Proposal T2 (Fiber " << model_index << "): " << t2 << " (" << add << ")";
    break;
  }
  case 2:
  {
    int model_index = rand()%new_params.m_NonFiberModelList.size();
    double t1 = new_params.m_NonFiberModelList[model_index]->GetT1();
    std::normal_distribution<double> normal_dist(0, t1*0.1*temperature);

    double add = 0;
    while (add == 0)
      add = normal_dist(randgen);

    if ( t1+add < new_params.m_NonFiberModelList[model_index]->GetT2() * 1.5 )
      add = -add;
    t1 += add;
    new_params.m_NonFiberModelList[model_index]->SetT1(t1);
    MITK_INFO << "Proposal T1 (Non-Fiber " << model_index << "): " << t1 << " (" << add << ")";
    break;
  }
  case 3:
  {
    int model_index = rand()%new_params.m_FiberModelList.size();
    double t1 = new_params.m_FiberModelList[model_index]->GetT1();
    std::normal_distribution<double> normal_dist(0, t1*0.1*temperature);

    double add = 0;
    while (add == 0)
      add = normal_dist(randgen);

    if ( t1+add < new_params.m_FiberModelList[model_index]->GetT2() * 1.5 )
      add = -add;
    t1 += add;
    new_params.m_FiberModelList[model_index]->SetT1(t1);
    MITK_INFO << "Proposal T1 (Fiber " << model_index << "): " << t1 << " (" << add << ")";
    break;
  }
  }

  return new_params;
}

double UpdateDiffusivity(double d, double temperature)
{
  std::random_device r;
  std::default_random_engine randgen(r());

  std::normal_distribution<double> normal_dist(0, d*0.1*temperature);

  double add = 0;
  while (add == 0)
    add = normal_dist(randgen);

  if (d+add > 0.0025)
    d -= add;
  else if ( d+add < 0.0 )
    d -= add;
  else
    d += add;

  return d;
}

void ProposeDiffusivities(mitk::DiffusionSignalModel<>* signalModel, double temperature)
{
  if (dynamic_cast<mitk::StickModel<>*>(signalModel))
  {
    mitk::StickModel<>* m = dynamic_cast<mitk::StickModel<>*>(signalModel);
    double new_d = UpdateDiffusivity(m->GetDiffusivity(), temperature);
    MITK_INFO << "d: " << new_d << " (" << new_d-m->GetDiffusivity() << ")";
    m->SetDiffusivity(new_d);
  }
  else  if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
  {
    mitk::TensorModel<>* m = dynamic_cast<mitk::TensorModel<>*>(signalModel);
    double new_d1 = UpdateDiffusivity(m->GetDiffusivity1(), temperature);
    double new_d2 = UpdateDiffusivity(m->GetDiffusivity2(), temperature);
    while (new_d1<new_d2)
      new_d2 = UpdateDiffusivity(m->GetDiffusivity2(), temperature);

    MITK_INFO << "d1: " << new_d1 << " (" << new_d1-m->GetDiffusivity1() << ")";
    MITK_INFO << "d2: " << new_d2 << " (" << new_d2-m->GetDiffusivity2() << ")";

    m->SetDiffusivity1(new_d1);
    m->SetDiffusivity2(new_d2);
    m->SetDiffusivity3(new_d2);
  }
  else  if (dynamic_cast<mitk::BallModel<>*>(signalModel))
  {
    mitk::BallModel<>* m = dynamic_cast<mitk::BallModel<>*>(signalModel);

    double new_d = UpdateDiffusivity(m->GetDiffusivity(), temperature);
    MITK_INFO << "d: " << new_d << " (" << new_d-m->GetDiffusivity() << ")";
    m->SetDiffusivity(new_d);
  }
  else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
  {
    mitk::AstroStickModel<>* m = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);

    double new_d = UpdateDiffusivity(m->GetDiffusivity(), temperature);
    MITK_INFO << "d: " << new_d << " (" << new_d-m->GetDiffusivity() << ")";
    m->SetDiffusivity(new_d);
  }
}

FiberfoxParameters MakeProposalDiff(FiberfoxParameters old_params, double temperature)
{
  FiberfoxParameters new_params(old_params);

  std::random_device r;
  std::default_random_engine randgen(r());

  std::uniform_int_distribution<int> uint1(0, new_params.m_NonFiberModelList.size() + new_params.m_FiberModelList.size() - 1);
  unsigned int prop = uint1(randgen);

  if (prop<new_params.m_NonFiberModelList.size())
  {
    MITK_INFO << "Proposal D (Non-Fiber " << prop << ")";
    ProposeDiffusivities( new_params.m_NonFiberModelList[prop], temperature );
  }
  else
  {
    prop -= new_params.m_NonFiberModelList.size();
    MITK_INFO << "Proposal D (Fiber " << prop  << ")";
    ProposeDiffusivities( new_params.m_FiberModelList[prop], temperature );
  }

  return new_params;
}

FiberfoxParameters MakeProposalVolume(double old_tdi_thr, double old_sqrt, double& new_tdi_thr, double& new_sqrt, FiberfoxParameters old_params, std::vector< itk::Image< double, 3 >::Pointer > frac, double temperature)
{
  FiberfoxParameters new_params(old_params);

  MITK_INFO << "Proposal Volume";
  std::random_device r;
  std::default_random_engine randgen(r());

  {
    std::normal_distribution<double> normal_dist(0, old_tdi_thr*0.1*temperature);
    new_tdi_thr = old_tdi_thr + normal_dist(randgen);
    while (new_tdi_thr<=0.01)
      new_tdi_thr = old_tdi_thr + normal_dist(randgen);
  }
  {
    std::normal_distribution<double> normal_dist(0, old_sqrt*0.1*temperature);
    new_sqrt = old_sqrt + normal_dist(randgen);
    while (new_sqrt<=0.01)
      new_sqrt = old_sqrt + normal_dist(randgen);
  }

  itk::TdiToVolumeFractionFilter< double >::Pointer fraction_generator = itk::TdiToVolumeFractionFilter< double >::New();
  fraction_generator->SetTdiThreshold(new_tdi_thr);
  fraction_generator->SetSqrt(new_sqrt);
  fraction_generator->SetInput(0, frac.at(0));
  fraction_generator->SetInput(1, frac.at(1));
  fraction_generator->SetInput(2, frac.at(2));
  fraction_generator->SetInput(3, frac.at(3));
  fraction_generator->SetInput(4, frac.at(4));
  fraction_generator->Update();

  new_params.m_FiberModelList[0]->SetVolumeFractionImage(fraction_generator->GetOutput(0));
  new_params.m_FiberModelList[1]->SetVolumeFractionImage(fraction_generator->GetOutput(1));
  new_params.m_NonFiberModelList[0]->SetVolumeFractionImage(fraction_generator->GetOutput(2));
  new_params.m_NonFiberModelList[1]->SetVolumeFractionImage(fraction_generator->GetOutput(3));

  MITK_INFO << "TDI Threshold: " << new_tdi_thr << " (" << new_tdi_thr-old_tdi_thr << ")";
  MITK_INFO << "SQRT: " << new_sqrt << " (" << new_sqrt-old_sqrt << ")";

  return new_params;
}

/*!
* \brief Command line interface to optimize Fiberfox parameters.
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("FiberfoxOptimization");
  parser.setCategory("Optimize Fiberfox Parameters");
  parser.setContributor("MIC");
  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("1. Mandatory Input:");
  parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameter File:", "fiberfox parameter file (.ffp)", us::Any(), false);
  parser.addArgument("tracts", "t", mitkCommandLineParser::String, "Input Tractogram:", "Input tractogram.", us::Any(), false);
  parser.addArgument("out_folder", "o", mitkCommandLineParser::String, "Output Folder:", "", us::Any(), false);
  parser.addArgument("dmri", "d", mitkCommandLineParser::String, "Target image:", "Target dMRI to approximate.", us::Any(), false);
  parser.addArgument("mask", "", mitkCommandLineParser::InputFile, "Mask image:", "Error is only calculated inside the mask image", false);
  parser.endGroup();

  parser.beginGroup("2. Parameters to optimize:");
  parser.addArgument("no_diff", "", mitkCommandLineParser::Bool, "Don't optimize diffusivities:", "Don't optimize diffusivities");
  parser.addArgument("no_relax", "", mitkCommandLineParser::Bool, "Don't optimize relaxation times:", "Don't optimize relaxation times");
  parser.addArgument("no_scale", "", mitkCommandLineParser::Bool, "Don't optimize signal scale:", "Don't optimize global signal scale");
  parser.endGroup();

  parser.beginGroup("3. Error measure:");
  parser.addArgument("fa_error", "", mitkCommandLineParser::Bool, "Optimize FA", "Optimize FA instead of raw signal. Requires FA image.");
  parser.addArgument("fa_image", "", mitkCommandLineParser::InputFile, "FA image:", "Weight error by FA histogram. Always necessary with option fa_error!");
  parser.addArgument("md_image", "", mitkCommandLineParser::InputFile, "MD image:", "Optimize MD in conjunction with FA (recommended when optimizing FA).");
  parser.endGroup();

  parser.beginGroup("4. Optimization of volume fraction maps:");
  parser.addArgument("tdi", "", mitkCommandLineParser::InputFile, "TDI:", "tract density image");
  parser.addArgument("wm", "", mitkCommandLineParser::InputFile, "WM:", "white matter volume fraction image");
  parser.addArgument("gm", "", mitkCommandLineParser::InputFile, "GM:", "gray matter volume fraction image");
  parser.addArgument("dgm", "", mitkCommandLineParser::InputFile, "DGM:", "subcortical gray matter volume fraction image");
  parser.addArgument("csf", "", mitkCommandLineParser::InputFile, "CSF:", "CSF volume fraction image");
  parser.addArgument("tdi_threshold", "", mitkCommandLineParser::Float, "", "", 0.75);
  parser.addArgument("sqrt", "", mitkCommandLineParser::Float, "", "", 1.0);
  parser.endGroup();

  parser.beginGroup("5. General parameters:");
  parser.addArgument("iterations", "", mitkCommandLineParser::Int, "Iterations:", "Number of optimizations steps", 1000);
  parser.addArgument("start_temp", "", mitkCommandLineParser::Float, "Start temperature:", "Higher temperature means larger parameter change proposals", 1.0);
  parser.addArgument("end_temp", "", mitkCommandLineParser::Float, "End temperature:", "Higher temperature means larger parameter change proposals", 0.1);
  parser.endGroup();

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string paramName = us::any_cast<std::string>(parsedArgs["parameters"]);
  std::string out_folder = us::any_cast<std::string>(parsedArgs["out_folder"]);
  std::string tract_file = us::any_cast<std::string>(parsedArgs["tracts"]);

  MITK_INFO << "Loading target dMRI and parameters";
  FiberfoxParameters parameters;
  parameters.LoadParameters(paramName);
  typedef itk::VectorImage< short, 3 >    ItkDwiType;
  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
  auto dwi = mitk::IOUtil::Load<mitk::Image>(us::any_cast<std::string>(parsedArgs["dmri"]), &functor);
  ItkDwiType::Pointer reference = mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi);
  parameters.m_SignalGen.m_ImageRegion = reference->GetLargestPossibleRegion();
  parameters.m_SignalGen.m_ImageSpacing = reference->GetSpacing();
  parameters.m_SignalGen.m_ImageOrigin = reference->GetOrigin();
  parameters.m_SignalGen.m_ImageDirection = reference->GetDirection();
  parameters.SetBvalue(static_cast<mitk::FloatProperty*>(dwi->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue());
  parameters.SetGradienDirections(static_cast<mitk::GradientDirectionsProperty*>( dwi->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer());

  auto tracts = mitk::IOUtil::Load<mitk::FiberBundle>(tract_file, &functor);

  int iterations=1000;
  if (parsedArgs.count("iterations"))
    iterations = us::any_cast<int>(parsedArgs["iterations"]);

  float start_temp=1.0;
  if (parsedArgs.count("start_temp"))
    start_temp = us::any_cast<float>(parsedArgs["start_temp"]);

  float end_temp=0.1;
  if (parsedArgs.count("end_temp"))
    end_temp = us::any_cast<float>(parsedArgs["end_temp"]);

  float tdi_threshold=0.75;
  if (parsedArgs.count("tdi_threshold"))
    tdi_threshold = us::any_cast<float>(parsedArgs["tdi_threshold"]);

  float sqrt=1.0;
  if (parsedArgs.count("sqrt"))
    sqrt = us::any_cast<float>(parsedArgs["sqrt"]);

  bool fa_error=false;
  if (parsedArgs.count("fa_error"))
    fa_error = true;

  std::string fa_file = "";
  if (parsedArgs.count("fa_image"))
    fa_file = us::any_cast<std::string>(parsedArgs["fa_image"]);

  std::string md_file = "";
  if (parsedArgs.count("md_image"))
    md_file = us::any_cast<std::string>(parsedArgs["md_image"]);

  std::vector< int > possible_proposals;
  if (!parsedArgs.count("no_diff"))
  {
    MITK_INFO << "Optimizing diffusivities";
    possible_proposals.push_back(0);
  }
  if (!parsedArgs.count("no_relax"))
  {
    MITK_INFO << "Optimizing relaxation constants";
    possible_proposals.push_back(1);
  }
  if (!parsedArgs.count("no_scale"))
  {
    MITK_INFO << "Optimizing global signal scale";
    possible_proposals.push_back(2);
  }
  if (possible_proposals.empty())
  {
    MITK_INFO << "Incompatible options. Nothing to optimize.";
    return EXIT_FAILURE;
  }

  itk::ImageFileReader< itk::Image< unsigned char, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< unsigned char, 3 > >::New();
  reader->SetFileName( us::any_cast<std::string>(parsedArgs["mask"]) );
  reader->Update();
  itk::Image< unsigned char,3 >::Pointer mask = reader->GetOutput();

  std::vector< itk::Image< double, 3 >::Pointer > fracs;
  if ( parsedArgs.count("tdi")>0 && parsedArgs.count("wm")>0 && parsedArgs.count("gm")>0 && parsedArgs.count("dgm")>0 && parsedArgs.count("csf")>0 )
  {
    MITK_INFO << "Optimizing volume fractions";
    {
      itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
      reader->SetFileName( us::any_cast<std::string>(parsedArgs["tdi"]) );
      reader->Update();
      fracs.push_back(reader->GetOutput());
    }
    {
      itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
      reader->SetFileName( us::any_cast<std::string>(parsedArgs["wm"]) );
      reader->Update();
      fracs.push_back(reader->GetOutput());
    }
    {
      itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
      reader->SetFileName( us::any_cast<std::string>(parsedArgs["gm"]) );
      reader->Update();
      fracs.push_back(reader->GetOutput());
    }
    {
      itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
      reader->SetFileName( us::any_cast<std::string>(parsedArgs["dgm"]) );
      reader->Update();
      fracs.push_back(reader->GetOutput());
    }
    {
      itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
      reader->SetFileName( us::any_cast<std::string>(parsedArgs["csf"]) );
      reader->Update();
      fracs.push_back(reader->GetOutput());
    }
    MITK_INFO << "Initial sqrt: " << sqrt;
    MITK_INFO << "Initial TDI threshold: " << tdi_threshold;

    possible_proposals.push_back(3);
  }

  std::vector< double > histogram_modifiers;
  itk::Image< double,3 >::Pointer fa_image = nullptr;
  if (fa_file.compare("")!=0)
  {
    itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
    reader->SetFileName( fa_file );
    reader->Update();
    fa_image = reader->GetOutput();

    int binsPerDimension = 20;
    using ImageToHistogramFilterType = itk::Statistics::MaskedImageToHistogramFilter< itk::Image< double,3 >, itk::Image< unsigned char,3 > >;

    ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
    lowerBound.Fill(0.0);

    ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
    upperBound.Fill(1.0);

    ImageToHistogramFilterType::HistogramType::SizeType size(1);
    size.Fill(binsPerDimension);

    ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    imageToHistogramFilter->SetInput( fa_image );
    imageToHistogramFilter->SetHistogramBinMinimum( lowerBound );
    imageToHistogramFilter->SetHistogramBinMaximum( upperBound );
    imageToHistogramFilter->SetHistogramSize( size );
    imageToHistogramFilter->SetMaskImage(mask);
    imageToHistogramFilter->SetMaskValue(1);
    imageToHistogramFilter->Update();

    ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();
    unsigned int max = 0;
    for(unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
    {
      if (histogram->GetFrequency(i)>max)
        max = histogram->GetFrequency(i);
    }
    MITK_INFO << "FA histogram modifiers:";
    for(unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
    {
      histogram_modifiers.push_back((double)max/histogram->GetFrequency(i));
      MITK_INFO << std::pow(histogram_modifiers.back(), 4);
    }
    if (fa_error)
      MITK_INFO << "Using FA error measure.";
  }
  itk::Image< double,3 >::Pointer md_image = nullptr;
  if (md_file.compare("")!=0)
  {
    itk::ImageFileReader< itk::Image< double, 3 > >::Pointer reader = itk::ImageFileReader< itk::Image< double, 3 > >::New();
    reader->SetFileName( md_file );
    reader->Update();
    md_image = reader->GetOutput();
    if (fa_error)
      MITK_INFO << "Using MD error measure.";
  }
  if (fa_error && fa_image.IsNull())
  {
    MITK_INFO << "Incompatible options. Need FA image to calculate FA error.";
    return EXIT_FAILURE;
  }

  itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
  tractsToDwiFilter->SetFiberBundle(tracts);
  tractsToDwiFilter->SetParameters(parameters);
  tractsToDwiFilter->Update();
  ItkDwiType::Pointer sim = tractsToDwiFilter->GetOutput();
  {
    mitk::Image::Pointer image = mitk::GrabItkImageMemory( tractsToDwiFilter->GetOutput() );
    image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(),
                                               mitk::GradientDirectionsProperty::New( parameters.m_SignalGen.GetGradientDirections() ) );
    image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(),
                                               mitk::FloatProperty::New( parameters.m_SignalGen.GetBvalue() ) );
    mitk::DiffusionPropertyHelper propertyHelper( image );
    propertyHelper.InitializeImage();
    mitk::IOUtil::Save(image, out_folder + "/initial.dwi");
  }


  double old_tdi_thr = tdi_threshold;
  double old_sqrt = sqrt;
  double new_tdi_thr;
  double new_sqrt;
  MITK_INFO << "\n\n";
  MITK_INFO << "Iterations: " << iterations;
  MITK_INFO << "start_temp: " << start_temp;
  MITK_INFO << "end_temp: " << end_temp;
  double alpha = log(end_temp/start_temp);
  int accepted = 0;

  double last_error = 9999999;
  if (fa_error)
  {
    MITK_INFO << "Calculating FA error";
    last_error = CalcErrorFA(histogram_modifiers, dwi, sim, mask, fa_image, md_image, true);
  }
  else
  {
    MITK_INFO << "Calculating raw-image error";
    last_error = CalcErrorSignal(histogram_modifiers, reference, sim, mask, fa_image);
  }
  MITK_INFO << "Initial E = " << last_error;
  MITK_INFO << "\n\n**************************************************************************************";

  std::random_device r;
  std::default_random_engine randgen(r());
  std::uniform_int_distribution<int> uint1(0, possible_proposals.size()-1);
  for (int i=0; i<iterations; ++i)
  {
    double temperature = start_temp * exp(alpha*(double)i/iterations);
    MITK_INFO << "Iteration " << i+1 << "/" << iterations << " (t=" << temperature << ")";

    FiberfoxParameters proposal(parameters);
    int select = uint1(randgen);
    switch (possible_proposals.at(select))
    {
    case 0:
    {
      proposal = MakeProposalDiff(proposal, temperature);
      break;
    }
    case 1:
    {
      proposal = MakeProposalRelaxation(proposal, temperature);
      break;
    }
    case 2:
    {
      proposal = MakeProposalScale(proposal, temperature);
      break;
    }
    case 3:
    {
      proposal = MakeProposalVolume(old_tdi_thr, old_sqrt, new_tdi_thr, new_sqrt, proposal, fracs, temperature);
      break;
    }
    }

    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());
    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    tractsToDwiFilter->SetFiberBundle(dynamic_cast<mitk::FiberBundle*>(tracts.GetPointer()));
    tractsToDwiFilter->SetParameters(proposal);
    tractsToDwiFilter->Update();
    ItkDwiType::Pointer sim = tractsToDwiFilter->GetOutput();
    std::cout.rdbuf (old);              // <-- restore

    double new_error = 9999999;
    if (fa_error && fa_image.IsNotNull())
      new_error = CalcErrorFA(histogram_modifiers, dwi, sim, mask, fa_image, md_image, true);
    else
      new_error = CalcErrorSignal(histogram_modifiers, reference, sim, mask, fa_image);
    MITK_INFO << "E = " << new_error << "(" << new_error-last_error << ")";
    if (last_error<new_error)
    {
      MITK_INFO << "Rejected (acc. rate " << (float)accepted/(i+1) << ")";
    }
    else
    {
      std::streambuf *old = cout.rdbuf(); // <-- save
      std::stringstream ss;
      std::cout.rdbuf (ss.rdbuf());
      mitk::Image::Pointer image = mitk::GrabItkImageMemory( tractsToDwiFilter->GetOutput() );
      image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(),
                                                 mitk::GradientDirectionsProperty::New( parameters.m_SignalGen.GetGradientDirections() ) );
      image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(),
                                                 mitk::FloatProperty::New( parameters.m_SignalGen.GetBvalue() ) );
      mitk::DiffusionPropertyHelper propertyHelper( image );
      propertyHelper.InitializeImage();
      mitk::IOUtil::Save(image, out_folder + "/optimized.dwi");

      proposal.SaveParameters(out_folder + "/optimized.ffp");
      std::cout.rdbuf (old);              // <-- restore

      accepted++;

      old_tdi_thr = new_tdi_thr;
      old_sqrt = new_sqrt;

      MITK_INFO << "Accepted (acc. rate " << (float)accepted/(i+1) << ")";
      parameters = FiberfoxParameters(proposal);
      last_error = new_error;
    }
    MITK_INFO << "\n\n\n";
  }

  return EXIT_SUCCESS;
}

