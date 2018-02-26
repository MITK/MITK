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

using namespace mitk;

float CompareDwi(itk::VectorImage< short, 3 >* dwi1, itk::VectorImage< short, 3 >* dwi2)
{
  typedef itk::VectorImage< short, 3 > DwiImageType;
  try{
    itk::ImageRegionIterator< DwiImageType > it1(dwi1, dwi1->GetLargestPossibleRegion());
    itk::ImageRegionIterator< DwiImageType > it2(dwi2, dwi2->GetLargestPossibleRegion());
    unsigned int count = 0;
    float difference = 0;
    while(!it1.IsAtEnd())
    {
      for (unsigned int i=0; i<dwi1->GetVectorLength(); ++i)
      {
        difference += abs(it1.Get()[i]-it2.Get()[i]);
        count++;
      }
      ++it1;
      ++it2;
    }
    return difference/count;
  }
  catch(...)
  {
    return -1;
  }
  return -1;
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
    float add = 0;
    while (add == 0)
      add =  normal_dist(randgen);
    new_params.m_SignalGen.m_SignalScale += add;
    MITK_INFO << "Proposal Signal Scale: " << add << " (" << new_params.m_SignalGen.m_SignalScale << ")";
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
    t2 += add;
    new_params.m_NonFiberModelList[model_index]->SetT2(t2);
    MITK_INFO << "Proposal T2 (Non-Fiber " << model_index << "): " << add << " (" << t2 << ")";
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
    t2 += add;
    new_params.m_FiberModelList[model_index]->SetT2(t2);
    MITK_INFO << "Proposal T2 (Fiber " << model_index << "): " << add << " (" << t2 << ")";
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
    t1 += add;
    new_params.m_NonFiberModelList[model_index]->SetT1(t1);
    MITK_INFO << "Proposal T1 (Non-Fiber " << model_index << "): " << add << " (" << t1 << ")";
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
    t1 += add;
    new_params.m_FiberModelList[model_index]->SetT1(t1);
    MITK_INFO << "Proposal T1 (Fiber " << model_index << "): " << add << " (" << t1 << ")";
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
  d += add;

  return d;
}

void ProposeDiffusivities(mitk::DiffusionSignalModel<>* signalModel, double temperature)
{
  if (dynamic_cast<mitk::StickModel<>*>(signalModel))
  {
    mitk::StickModel<>* m = dynamic_cast<mitk::StickModel<>*>(signalModel);
    m->SetDiffusivity(UpdateDiffusivity(m->GetDiffusivity(), temperature));
    MITK_INFO << "d: " << m->GetDiffusivity();
  }
  else  if (dynamic_cast<mitk::TensorModel<>*>(signalModel))
  {
    mitk::TensorModel<>* m = dynamic_cast<mitk::TensorModel<>*>(signalModel);
    m->SetDiffusivity1(UpdateDiffusivity(m->GetDiffusivity1(), temperature));
    double new_d2 = UpdateDiffusivity(m->GetDiffusivity2(), temperature);
    while (m->GetDiffusivity1()*0.9<new_d2)
      new_d2 = UpdateDiffusivity(m->GetDiffusivity2(), temperature);
    m->SetDiffusivity2(new_d2);
    m->SetDiffusivity3(new_d2);
    MITK_INFO << "d1: " << m->GetDiffusivity1();
    MITK_INFO << "d2: " << m->GetDiffusivity2();
    MITK_INFO << "d3: " << m->GetDiffusivity3();
  }
  else  if (dynamic_cast<mitk::BallModel<>*>(signalModel))
  {
    mitk::BallModel<>* m = dynamic_cast<mitk::BallModel<>*>(signalModel);
    m->SetDiffusivity(UpdateDiffusivity(m->GetDiffusivity(), temperature));
    MITK_INFO << "d: " << m->GetDiffusivity();
  }
  else if (dynamic_cast<mitk::AstroStickModel<>*>(signalModel))
  {
    mitk::AstroStickModel<>* m = dynamic_cast<mitk::AstroStickModel<>*>(signalModel);
    m->SetDiffusivity(UpdateDiffusivity(m->GetDiffusivity(), temperature));
    MITK_INFO << "d: " << m->GetDiffusivity();
  }
}

FiberfoxParameters MakeProposalDiff(FiberfoxParameters old_params, double temperature)
{
  std::random_device r;
  std::default_random_engine randgen(r());

  std::uniform_int_distribution<int> uint1(0, 1);

  FiberfoxParameters new_params(old_params);
  int prop = uint1(randgen);
  switch(prop)
  {
  case 0:
  {
    int model_index = rand()%new_params.m_NonFiberModelList.size();
    ProposeDiffusivities( new_params.m_NonFiberModelList[model_index], temperature );
    MITK_INFO << "Proposal D (Non-Fiber " << model_index << ")";
    break;
  }
  case 1:
  {
    int model_index = rand()%new_params.m_FiberModelList.size();
    ProposeDiffusivities( new_params.m_FiberModelList[model_index], temperature );
    MITK_INFO << "Proposal D (Fiber " << model_index << ")";
    break;
  }
  }

  return new_params;
}

/*!
* \brief Command line interface to Fiberfox.
* Simulate a diffusion-weighted image from a tractogram using the specified parameter file.
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("FiberfoxOptimization");
  parser.setCategory("Optimize Fiberfox Parameters");
  parser.setContributor("MIC");
  parser.setDescription("Command line interface to Fiberfox." " Simulate a diffusion-weighted image from a tractogram using the specified parameter file.");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameter file:", "fiberfox parameter file (.ffp)", us::Any(), false);
  parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:", "Input tractogram or diffusion-weighted image.", us::Any(), false);
  parser.addArgument("template", "t", mitkCommandLineParser::String, "Template image:", "Use parameters of the template diffusion-weighted image.", us::Any(), false);
  parser.addArgument("iterations", "", mitkCommandLineParser::Int, "Iterations:", "Number of optimizations steps", 1000);
  parser.addArgument("start_temp", "", mitkCommandLineParser::Float, "Start temperature:", "", 1.0);
  parser.addArgument("end_temp", "", mitkCommandLineParser::Float, "End temperature:", "", 0.1);
  parser.addArgument("no_diff", "", mitkCommandLineParser::Bool, "Don't optimize diffusivities:", "Don't optimize diffusivities");
  parser.addArgument("no_relax", "", mitkCommandLineParser::Bool, "Don't optimize relaxation times and signal scale:", "Don't optimize relaxation times and signal scale");

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

  if (no_relax && no_diff)
  {
    MITK_INFO << "Incompatible options. Nothing to optimize.";
    return EXIT_FAILURE;
  }

  FiberfoxParameters parameters;
  parameters.LoadParameters(paramName);

  MITK_INFO << "Loading template image";
  typedef itk::VectorImage< short, 3 >    ItkDwiType;
  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
  mitk::Image::Pointer dwi = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(us::any_cast<std::string>(parsedArgs["template"]), &functor)[0].GetPointer());
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

  MITK_INFO << "Iterations: " << iterations;
  MITK_INFO << "start_temp: " << start_temp;
  MITK_INFO << "end_temp: " << end_temp;
  double alpha = log(end_temp/start_temp);
  int accepted = 0;

  float last_diff =  CompareDwi(sim, reference);
  for (int i=0; i<1000; ++i)
  {
    double temperature = start_temp * exp(alpha*(double)i/iterations);
    MITK_INFO << "Temperature: " << temperature << " (" << i+1 << "/" << iterations << ")";

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

    float diff = CompareDwi(sim, reference);
    if (last_diff<diff)
    {
      MITK_INFO << "Rejected proposal (acc. rate " << (float)accepted/(i+1) << ")";
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

      MITK_INFO << "Accepted proposal (acc. rate " << (float)accepted/(i+1) << ")";
      parameters = FiberfoxParameters(proposal);
      last_diff = diff;
    }
    MITK_INFO << "\n\n\n";
  }

  return EXIT_SUCCESS;
}

