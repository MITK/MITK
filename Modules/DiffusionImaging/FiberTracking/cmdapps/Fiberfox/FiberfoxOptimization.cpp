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
#include <boost/lexical_cast.hpp>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itksys/SystemTools.hxx>
#include <mitkFiberfoxParameters.h>
#include <random>
#include <mitkTensorImage.h>
#include <itkTensorDerivedMeasurementsFilter.h>
#include <itkAdcImageFilter.h>
#include <itkMaskedImageToHistogramFilter.h>

using namespace mitk;

double CalcErrorSignal(itk::VectorImage< short, 3 >* reference, itk::VectorImage< short, 3 >* simulation, itk::Image< unsigned char,3 >::Pointer mask)
{
  typedef itk::VectorImage< short, 3 > DwiImageType;
  try
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
  catch(...)
  {
    return -1;
  }
  return -1;
}

double CalcErrorFA(const std::vector<double>& histo_mod, mitk::Image::Pointer dwi1, itk::VectorImage< short, 3 >* dwi2, itk::Image< unsigned char,3 >::Pointer mask, itk::Image< double,3 >::Pointer fa1, itk::Image< double,3 >::Pointer md1)
{
  typedef itk::TensorDerivedMeasurementsFilter<double> MeasurementsType;
  typedef itk::Image< double, 3 > DoubleImageType;

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

          double fa_diff = fabs(it2.Get()/fa - 1.0);
          double md_diff = fabs(it4.Get()/it3.Get() - 1.0);
          error += mod*mod*mod*mod * (fa_diff + md_diff);
          count += 2;
        }
      }
      ++it1;
      ++it2;
      ++it3;
      ++it4;
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
          error += mod * fa_diff;
          ++count;
        }
      }
      ++it1;
      ++it2;
    }
  }

  return error/count;
}

FiberfoxParameters MakeProposalRelaxation(FiberfoxParameters old_params, double temperature)
{
  std::random_device r;
  std::default_random_engine randgen(r());

  std::uniform_int_distribution<int> uint1(0, 4);

  FiberfoxParameters new_params(old_params);
  int prop = uint1(randgen);

  switch(prop)
  {
  case 0:
  {
    std::normal_distribution<double> normal_dist(0, new_params.m_SignalGen.m_SignalScale*0.1*temperature);
    double add = 0;
    while (add == 0)
      add =  normal_dist(randgen);
    new_params.m_SignalGen.m_SignalScale += add;
    MITK_INFO << "Proposal Signal Scale: " << new_params.m_SignalGen.m_SignalScale << " (" << add << ")";
    break;
  }
  case 1:
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
  case 2:
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
  case 3:
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
  case 4:
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
  parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameter file:", "fiberfox parameter file (.ffp)", us::Any(), false);
  parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:", "Input tractogram or diffusion-weighted image.", us::Any(), false);
  parser.addArgument("target", "t", mitkCommandLineParser::String, "Target image:", "Approximate target dMRI.", us::Any(), false);
  parser.addArgument("mask", "", mitkCommandLineParser::InputFile, "Mask image:", "Error is only calculated inside the mask image", false);

  parser.addArgument("fa_image", "", mitkCommandLineParser::InputFile, "FA image", "Optimize FA instead of raw signal");
  parser.addArgument("md_image", "", mitkCommandLineParser::InputFile, "MD image", "Optimize MD in conjunction with FA (recommended when optimizing FA)");

  parser.addArgument("no_diff", "", mitkCommandLineParser::Bool, "Don't optimize diffusivities:", "Don't optimize diffusivities");
  parser.addArgument("no_relax", "", mitkCommandLineParser::Bool, "Don't optimize relaxation times and signal scale:", "Don't optimize relaxation times and signal scale");

  parser.addArgument("iterations", "", mitkCommandLineParser::Int, "Iterations:", "Number of optimizations steps", 1000);
  parser.addArgument("start_temp", "", mitkCommandLineParser::Float, "Start temperature:", "", 1.0);
  parser.addArgument("end_temp", "", mitkCommandLineParser::Float, "End temperature:", "", 0.1);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string paramName = us::any_cast<std::string>(parsedArgs["parameters"]);

  std::string input = us::any_cast<std::string>(parsedArgs["input"]);

  int iterations=1000;
  if (parsedArgs.count("iterations"))
    iterations = us::any_cast<int>(parsedArgs["iterations"]);

  float start_temp=1.0;
  if (parsedArgs.count("start_temp"))
    start_temp = us::any_cast<float>(parsedArgs["start_temp"]);

  float end_temp=0.1;
  if (parsedArgs.count("end_temp"))
    end_temp = us::any_cast<float>(parsedArgs["end_temp"]);

  bool no_diff=false;
  if (parsedArgs.count("no_diff"))
    no_diff = true;

  bool no_relax=false;
  if (parsedArgs.count("no_relax"))
    no_relax = true;

  std::string fa_file = "";
  if (parsedArgs.count("fa_image"))
    fa_file = us::any_cast<std::string>(parsedArgs["fa_image"]);

  std::string md_file = "";
  if (parsedArgs.count("md_image"))
    md_file = us::any_cast<std::string>(parsedArgs["md_image"]);

  std::string mask_file = "";
  if (parsedArgs.count("mask"))
    mask_file = us::any_cast<std::string>(parsedArgs["mask"]);

  if (no_relax && no_diff)
  {
    MITK_INFO << "Incompatible options. Nothing to optimize.";
    return EXIT_FAILURE;
  }

  itk::Image< unsigned char,3 >::Pointer mask = nullptr;
  if (mask_file.compare("")!=0)
  {
    mitk::Image::Pointer mitk_mask = mitk::IOUtil::Load<mitk::Image>(mask_file);
    mitk::CastToItkImage(mitk_mask, mask);
  }

  std::vector< double > histogram_modifiers;
  itk::Image< double,3 >::Pointer fa_image = nullptr;
  if (fa_file.compare("")!=0)
  {
    mitk::Image::Pointer mitk_img = mitk::IOUtil::Load<mitk::Image>(fa_file);
    mitk::CastToItkImage(mitk_img, fa_image);

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
    for(unsigned int i = 0; i < histogram->GetSize()[0]; ++i)
    {
      histogram_modifiers.push_back((double)max/histogram->GetFrequency(i));
      MITK_INFO << histogram_modifiers.back();
    }
  }

  itk::Image< double,3 >::Pointer md_image = nullptr;
  if (md_file.compare("")!=0)
  {
    mitk::Image::Pointer mitk_img = mitk::IOUtil::Load<mitk::Image>(md_file);
    mitk::CastToItkImage(mitk_img, md_image);
  }

  FiberfoxParameters parameters;
  parameters.LoadParameters(paramName);

  MITK_INFO << "Loading target image";
  typedef itk::VectorImage< short, 3 >    ItkDwiType;
  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
  mitk::Image::Pointer dwi = mitk::IOUtil::Load<mitk::Image>(us::any_cast<std::string>(parsedArgs["target"]), &functor);
  ItkDwiType::Pointer reference = mitk::DiffusionPropertyHelper::GetItkVectorImage(dwi);
  parameters.m_SignalGen.m_ImageRegion = reference->GetLargestPossibleRegion();
  parameters.m_SignalGen.m_ImageSpacing = reference->GetSpacing();
  parameters.m_SignalGen.m_ImageOrigin = reference->GetOrigin();
  parameters.m_SignalGen.m_ImageDirection = reference->GetDirection();
  parameters.SetBvalue(static_cast<mitk::FloatProperty*>(dwi->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue());
  parameters.SetGradienDirections(static_cast<mitk::GradientDirectionsProperty*>( dwi->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer());

  mitk::BaseData::Pointer inputData = mitk::IOUtil::Load(input, &functor)[0];

  itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
  tractsToDwiFilter->SetFiberBundle(dynamic_cast<mitk::FiberBundle*>(inputData.GetPointer()));
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
    mitk::IOUtil::Save(image, "initial.dwi");
  }

  MITK_INFO << "\n\n";
  MITK_INFO << "Iterations: " << iterations;
  MITK_INFO << "start_temp: " << start_temp;
  MITK_INFO << "end_temp: " << end_temp;
  double alpha = log(end_temp/start_temp);
  int accepted = 0;

  double last_error = 9999999;
  if (fa_image.IsNotNull())
  {
    MITK_INFO << "Calculating FA error";
    last_error = CalcErrorFA(histogram_modifiers, dwi, sim, mask, fa_image, md_image);
  }
  else
  {
    MITK_INFO << "Calculating raw-image error";
    last_error = CalcErrorSignal(reference, sim, mask);
  }
  MITK_INFO << "Initial E = " << last_error;
  MITK_INFO << "\n\n**************************************************************************************";

  for (int i=0; i<iterations; ++i)
  {
    double temperature = start_temp * exp(alpha*(double)i/iterations);
    MITK_INFO << "Iteration " << i+1 << "/" << iterations << " (t=" << temperature << ")";

    std::random_device r;
    std::default_random_engine randgen(r());
    std::uniform_int_distribution<int> uint1(0, 1);

    FiberfoxParameters proposal(parameters);
    int select = uint1(randgen);
    if (no_relax)
      select = 0;
    else if (no_diff)
      select = 1;

    if (select==0)
      proposal = MakeProposalDiff(proposal, temperature);
    else
      proposal = MakeProposalRelaxation(proposal, temperature);

    std::streambuf *old = cout.rdbuf(); // <-- save
    std::stringstream ss;
    std::cout.rdbuf (ss.rdbuf());
    itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();
    tractsToDwiFilter->SetFiberBundle(dynamic_cast<mitk::FiberBundle*>(inputData.GetPointer()));
    tractsToDwiFilter->SetParameters(proposal);
    tractsToDwiFilter->Update();
    ItkDwiType::Pointer sim = tractsToDwiFilter->GetOutput();
    std::cout.rdbuf (old);              // <-- restore

    double new_error = 9999999;
    if (fa_image.IsNotNull())
      new_error = CalcErrorFA(histogram_modifiers, dwi, sim, mask, fa_image, md_image);
    else
      new_error = CalcErrorSignal(reference, sim, mask);
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
      mitk::IOUtil::Save(image, "optimized.dwi");

      proposal.SaveParameters("optimized.ffp");
      std::cout.rdbuf (old);              // <-- restore

      accepted++;

      MITK_INFO << "Accepted (acc. rate " << (float)accepted/(i+1) << ")";
      parameters = FiberfoxParameters(proposal);
      last_error = new_error;
    }
    MITK_INFO << "\n\n\n";
  }

  return EXIT_SUCCESS;
}

