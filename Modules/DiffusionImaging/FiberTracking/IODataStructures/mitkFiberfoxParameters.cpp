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
#include <boost/lexical_cast.hpp>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>

template< class ScalarType >
mitk::FiberfoxParameters< ScalarType >::FiberfoxParameters()
    : m_NoiseModel(NULL)
{

}

template< class ScalarType >
mitk::FiberfoxParameters< ScalarType >::~FiberfoxParameters()
{
    //    if (m_NoiseModel!=NULL)
    //        delete m_NoiseModel;
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
    double C = sqrt(4*M_PI);
    phi(0) = 0.0;
    phi(NPoints-1) = 0.0;
    for(int i=0; i<NPoints; i++)
    {
        theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - M_PI / 2.0;
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

mitk::SignalGenerationParameters::GradientType mitk::SignalGenerationParameters::GetGradientDirection(unsigned int i)
{
    return m_GradientDirections.at(i);
}

void mitk::SignalGenerationParameters::SetNumWeightedVolumes(int numGradients)
{
    m_NumGradients = numGradients;
    GenerateGradientHalfShell();
}

void mitk::SignalGenerationParameters::SetGradienDirections(GradientListType gradientList)
{
    m_GradientDirections = gradientList;
    m_NumGradients = 0;
    m_NumBaseline = 0;
    for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
    {
        if (m_GradientDirections.at(i).GetNorm()>0.0001)
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

        if (m_GradientDirections.at(i).GetNorm()>0.0001)
            m_NumGradients++;
        else
            m_NumBaseline++;
    }
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::SaveParameters(string filename)
{
    if(filename.empty())
        return;
    if(".ffp"!=filename.substr(filename.size()-4, 4))
        filename += ".ffp";

    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

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
    parameters.put("fiberfox.image.basic.direction.1", m_SignalGen.m_ImageDirection[0][0]);
    parameters.put("fiberfox.image.basic.direction.2", m_SignalGen.m_ImageDirection[0][1]);
    parameters.put("fiberfox.image.basic.direction.3", m_SignalGen.m_ImageDirection[0][2]);
    parameters.put("fiberfox.image.basic.direction.4", m_SignalGen.m_ImageDirection[1][0]);
    parameters.put("fiberfox.image.basic.direction.5", m_SignalGen.m_ImageDirection[1][1]);
    parameters.put("fiberfox.image.basic.direction.6", m_SignalGen.m_ImageDirection[1][2]);
    parameters.put("fiberfox.image.basic.direction.7", m_SignalGen.m_ImageDirection[2][0]);
    parameters.put("fiberfox.image.basic.direction.8", m_SignalGen.m_ImageDirection[2][1]);
    parameters.put("fiberfox.image.basic.direction.9", m_SignalGen.m_ImageDirection[2][2]);
    parameters.put("fiberfox.image.basic.numgradients", m_SignalGen.GetNumWeightedVolumes());
    for( unsigned int i=0; i<this->m_SignalGen.GetNumVolumes(); i++)
    {
        parameters.put("fiberfox.image.gradients."+boost::lexical_cast<string>(i)+".x", m_SignalGen.GetGradientDirection(i)[0]);
        parameters.put("fiberfox.image.gradients."+boost::lexical_cast<string>(i)+".y", m_SignalGen.GetGradientDirection(i)[1]);
        parameters.put("fiberfox.image.gradients."+boost::lexical_cast<string>(i)+".z", m_SignalGen.GetGradientDirection(i)[2]);
    }

    parameters.put("fiberfox.image.acquisitiontype", m_SignalGen.m_AcquisitionType);
    parameters.put("fiberfox.image.coilsensitivityprofile", m_SignalGen.m_CoilSensitivityProfile);
    parameters.put("fiberfox.image.numberofcoils", m_SignalGen.m_NumberOfCoils);
    parameters.put("fiberfox.image.reversephase", m_SignalGen.m_ReversePhase);
    parameters.put("fiberfox.image.partialfourier", m_SignalGen.m_PartialFourier);
    parameters.put("fiberfox.image.noisevariance", m_SignalGen.m_NoiseVariance);
    parameters.put("fiberfox.image.trep", m_SignalGen.m_tRep);
    parameters.put("fiberfox.image.signalScale", m_SignalGen.m_SignalScale);
    parameters.put("fiberfox.image.tEcho", m_SignalGen.m_tEcho);
    parameters.put("fiberfox.image.tLine", m_SignalGen.m_tLine);
    parameters.put("fiberfox.image.tInhom", m_SignalGen.m_tInhom);
    parameters.put("fiberfox.image.bvalue", m_SignalGen.m_Bvalue);
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
    parameters.put("fiberfox.image.artifacts.addringing", m_SignalGen.m_DoAddGibbsRinging);
    parameters.put("fiberfox.image.artifacts.doAddMotion", m_SignalGen.m_DoAddMotion);
    parameters.put("fiberfox.image.artifacts.randomMotion", m_SignalGen.m_DoRandomizeMotion);
    parameters.put("fiberfox.image.artifacts.translation0", m_SignalGen.m_Translation[0]);
    parameters.put("fiberfox.image.artifacts.translation1", m_SignalGen.m_Translation[1]);
    parameters.put("fiberfox.image.artifacts.translation2", m_SignalGen.m_Translation[2]);
    parameters.put("fiberfox.image.artifacts.rotation0", m_SignalGen.m_Rotation[0]);
    parameters.put("fiberfox.image.artifacts.rotation1", m_SignalGen.m_Rotation[1]);
    parameters.put("fiberfox.image.artifacts.rotation2", m_SignalGen.m_Rotation[2]);
    parameters.put("fiberfox.image.artifacts.motionvolumes", m_Misc.m_MotionVolumesBox);
    parameters.put("fiberfox.image.artifacts.addnoise", m_Misc.m_CheckAddNoiseBox);
    parameters.put("fiberfox.image.artifacts.addghosts", m_Misc.m_CheckAddGhostsBox);
    parameters.put("fiberfox.image.artifacts.addaliasing", m_Misc.m_CheckAddAliasingBox);
    parameters.put("fiberfox.image.artifacts.addspikes", m_Misc.m_CheckAddSpikesBox);
    parameters.put("fiberfox.image.artifacts.addeddycurrents", m_Misc.m_CheckAddEddyCurrentsBox);
    parameters.put("fiberfox.image.artifacts.doAddDistortions", m_Misc.m_CheckAddDistortionsBox);
    parameters.put("fiberfox.image.outputvolumefractions", m_Misc.m_CheckOutputVolumeFractionsBox);
    parameters.put("fiberfox.image.showadvanced", m_Misc.m_CheckAdvancedSignalOptionsBox);
    parameters.put("fiberfox.image.signalmodelstring", m_Misc.m_SignalModelString);
    parameters.put("fiberfox.image.artifactmodelstring", m_Misc.m_ArtifactModelString);
    parameters.put("fiberfox.image.outpath", m_Misc.m_OutputPath);
    parameters.put("fiberfox.fibers.realtime", m_Misc.m_CheckRealTimeFibersBox);
    parameters.put("fiberfox.fibers.showadvanced", m_Misc.m_CheckAdvancedFiberOptionsBox);
    parameters.put("fiberfox.fibers.constantradius", m_Misc.m_CheckConstantRadiusBox);
    parameters.put("fiberfox.fibers.includeFiducials", m_Misc.m_CheckIncludeFiducialsBox);

    if (m_NoiseModel!=NULL)
    {
        parameters.put("fiberfox.image.artifacts.noisevariance", m_NoiseModel->GetNoiseVariance());
        if (dynamic_cast<mitk::RicianNoiseModel<ScalarType>*>(m_NoiseModel.get()))
            parameters.put("fiberfox.image.artifacts.noisetype", "rice");
        else if (dynamic_cast<mitk::ChiSquareNoiseModel<ScalarType>*>(m_NoiseModel.get()))
            parameters.put("fiberfox.image.artifacts.noisetype", "chisquare");
    }

    for (int i=0; i<m_FiberModelList.size()+m_NonFiberModelList.size(); i++)
    {
        mitk::DiffusionSignalModel<ScalarType>* signalModel = NULL;
        if (i<m_FiberModelList.size())
        {
            signalModel = m_FiberModelList.at(i);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".type", "fiber");
        }
        else
        {
            signalModel = m_NonFiberModelList.at(i-m_FiberModelList.size());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".type", "non-fiber");
        }

        if (dynamic_cast<mitk::StickModel<ScalarType>*>(signalModel))
        {
            mitk::StickModel<ScalarType>* model = dynamic_cast<mitk::StickModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".model", "stick");
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".d", model->GetDiffusivity());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t2", model->GetT2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t1", model->GetT1());
        }
        else  if (dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel))
        {
            mitk::TensorModel<ScalarType>* model = dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".model", "tensor");
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".d1", model->GetDiffusivity1());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".d2", model->GetDiffusivity2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".d3", model->GetDiffusivity3());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t2", model->GetT2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t1", model->GetT1());
        }
        else  if (dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel))
        {
            mitk::RawShModel<ScalarType>* model = dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".model", "prototype");
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".minFA", model->GetFaRange().first);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".maxFA", model->GetFaRange().second);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".minADC", model->GetAdcRange().first);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".maxADC", model->GetAdcRange().second);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".maxNumSamples", model->GetMaxNumKernels());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".numSamples", model->GetNumberOfKernels());
            int shOrder = model->GetShOrder();
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".numCoeffs", (shOrder*shOrder + shOrder + 2)/2 + shOrder);

            for (unsigned int j=0; j<model->GetNumberOfKernels(); j++)
            {
                vnl_vector< double > coeffs = model->GetCoefficients(j);
                for (unsigned int k=0; k<coeffs.size(); k++)
                    parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".kernels."+boost::lexical_cast<string>(j)+".coeffs."+boost::lexical_cast<string>(k), coeffs[k]);
                parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".kernels."+boost::lexical_cast<string>(j)+".B0", model->GetBaselineSignal(j));
            }
        }
        else  if (dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel))
        {
            mitk::BallModel<ScalarType>* model = dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".model", "ball");
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".d", model->GetDiffusivity());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t2", model->GetT2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t1", model->GetT1());
        }
        else  if (dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel))
        {
            mitk::AstroStickModel<ScalarType>* model = dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".model", "astrosticks");
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".d", model->GetDiffusivity());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t2", model->GetT2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t1", model->GetT1());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".randomize", model->GetRandomizeSticks());
        }
        else  if (dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel))
        {
            mitk::DotModel<ScalarType>* model = dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".model", "dot");
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t2", model->GetT2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".t1", model->GetT1());
        }

        if (signalModel!=NULL)
        {
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".ID", signalModel->m_CompartmentId);

            if (signalModel->GetVolumeFractionImage().IsNotNull())
            {
                try{
                    itk::ImageFileWriter<ItkDoubleImgType>::Pointer writer = itk::ImageFileWriter<ItkDoubleImgType>::New();
                    writer->SetFileName(filename+"_VOLUME"+boost::lexical_cast<string>(signalModel->m_CompartmentId)+".nrrd");
                    writer->SetInput(signalModel->GetVolumeFractionImage());
                    writer->Update();
                    MITK_INFO << "Volume fraction image for compartment "+boost::lexical_cast<string>(signalModel->m_CompartmentId)+" saved.";
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
        itk::ImageFileWriter<ItkDoubleImgType>::Pointer writer = itk::ImageFileWriter<ItkDoubleImgType>::New();
        writer->SetFileName(filename+"_FMAP.nrrd");
        writer->SetInput(m_SignalGen.m_FrequencyMap);
        writer->Update();
    }
    catch(...)
    {
        MITK_INFO << "No frequency map saved.";
    }
    try{
        itk::ImageFileWriter<ItkUcharImgType>::Pointer writer = itk::ImageFileWriter<ItkUcharImgType>::New();
        writer->SetFileName(filename+"_MASK.nrrd");
        writer->SetInput(m_SignalGen.m_MaskImage);
        writer->Update();
    }
    catch(...)
    {
        MITK_INFO << "No mask image saved.";
    }

    setlocale(LC_ALL, currLocale.c_str());
}


template< class ScalarType >
template< class ParameterType >
ParameterType mitk::FiberfoxParameters< ScalarType >::ReadVal(boost::property_tree::ptree::value_type const& v, std::string tag, ParameterType defaultValue, bool essential)
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

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::LoadParameters(string filename)
{
    m_MissingTags = "";
    if(filename.empty())
        return;

    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, NULL );

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
    boost::property_tree::xml_parser::read_xml(filename, parameterTree);


    m_FiberModelList.clear();
    m_NonFiberModelList.clear();
    if (m_NoiseModel)
        m_NoiseModel = nullptr;

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
            m_Misc.m_SignalModelString = ReadVal<string>(v1,"signalmodelstring", m_Misc.m_SignalModelString);
            m_Misc.m_ArtifactModelString = ReadVal<string>(v1,"artifactmodelstring", m_Misc.m_ArtifactModelString);
            m_Misc.m_OutputPath = ReadVal<string>(v1,"outpath", m_Misc.m_OutputPath);
            m_Misc.m_CheckOutputVolumeFractionsBox = ReadVal<bool>(v1,"outputvolumefractions", m_Misc.m_CheckOutputVolumeFractionsBox);
            m_Misc.m_CheckAdvancedSignalOptionsBox = ReadVal<bool>(v1,"showadvanced", m_Misc.m_CheckAdvancedSignalOptionsBox);
            m_Misc.m_CheckAddDistortionsBox = ReadVal<bool>(v1,"artifacts.doAddDistortions", m_Misc.m_CheckAddDistortionsBox);
            m_Misc.m_CheckAddNoiseBox = ReadVal<bool>(v1,"artifacts.addnoise", m_Misc.m_CheckAddNoiseBox);
            m_Misc.m_CheckAddGhostsBox = ReadVal<bool>(v1,"artifacts.addghosts", m_Misc.m_CheckAddGhostsBox);
            m_Misc.m_CheckAddAliasingBox = ReadVal<bool>(v1,"artifacts.addaliasing", m_Misc.m_CheckAddAliasingBox);
            m_Misc.m_CheckAddSpikesBox = ReadVal<bool>(v1,"artifacts.addspikes", m_Misc.m_CheckAddSpikesBox);
            m_Misc.m_CheckAddEddyCurrentsBox = ReadVal<bool>(v1,"artifacts.addeddycurrents", m_Misc.m_CheckAddEddyCurrentsBox);

            m_SignalGen.m_ImageRegion.SetSize(0, ReadVal<int>(v1,"basic.size.x",m_SignalGen.m_ImageRegion.GetSize(0)));
            m_SignalGen.m_ImageRegion.SetSize(1, ReadVal<int>(v1,"basic.size.y",m_SignalGen.m_ImageRegion.GetSize(1)));
            m_SignalGen.m_ImageRegion.SetSize(2, ReadVal<int>(v1,"basic.size.z",m_SignalGen.m_ImageRegion.GetSize(2)));
            m_SignalGen.m_ImageSpacing[0] = ReadVal<double>(v1,"basic.spacing.x",m_SignalGen.m_ImageSpacing[0]);
            m_SignalGen.m_ImageSpacing[1] = ReadVal<double>(v1,"basic.spacing.y",m_SignalGen.m_ImageSpacing[1]);
            m_SignalGen.m_ImageSpacing[2] = ReadVal<double>(v1,"basic.spacing.z",m_SignalGen.m_ImageSpacing[2]);
            m_SignalGen.m_ImageOrigin[0] = ReadVal<double>(v1,"basic.origin.x",m_SignalGen.m_ImageOrigin[0]);
            m_SignalGen.m_ImageOrigin[1] = ReadVal<double>(v1,"basic.origin.y",m_SignalGen.m_ImageOrigin[1]);
            m_SignalGen.m_ImageOrigin[2] = ReadVal<double>(v1,"basic.origin.z",m_SignalGen.m_ImageOrigin[2]);
            m_SignalGen.m_ImageDirection[0][0] = ReadVal<double>(v1,"basic.direction.1",m_SignalGen.m_ImageDirection[0][0]);
            m_SignalGen.m_ImageDirection[0][1] = ReadVal<double>(v1,"basic.direction.2",m_SignalGen.m_ImageDirection[0][1]);
            m_SignalGen.m_ImageDirection[0][2] = ReadVal<double>(v1,"basic.direction.3",m_SignalGen.m_ImageDirection[0][2]);
            m_SignalGen.m_ImageDirection[1][0] = ReadVal<double>(v1,"basic.direction.4",m_SignalGen.m_ImageDirection[1][0]);
            m_SignalGen.m_ImageDirection[1][1] = ReadVal<double>(v1,"basic.direction.5",m_SignalGen.m_ImageDirection[1][1]);
            m_SignalGen.m_ImageDirection[1][2] = ReadVal<double>(v1,"basic.direction.6",m_SignalGen.m_ImageDirection[1][2]);
            m_SignalGen.m_ImageDirection[2][0] = ReadVal<double>(v1,"basic.direction.7",m_SignalGen.m_ImageDirection[2][0]);
            m_SignalGen.m_ImageDirection[2][1] = ReadVal<double>(v1,"basic.direction.8",m_SignalGen.m_ImageDirection[2][1]);
            m_SignalGen.m_ImageDirection[2][2] = ReadVal<double>(v1,"basic.direction.9",m_SignalGen.m_ImageDirection[2][2]);

            m_SignalGen.m_AcquisitionType = (SignalGenerationParameters::AcquisitionType)ReadVal<int>(v1,"acquisitiontype", m_SignalGen.m_AcquisitionType);
            m_SignalGen.m_CoilSensitivityProfile = (SignalGenerationParameters::CoilSensitivityProfile)ReadVal<int>(v1,"coilsensitivityprofile", m_SignalGen.m_CoilSensitivityProfile);
            m_SignalGen.m_NumberOfCoils = ReadVal<int>(v1,"numberofcoils", m_SignalGen.m_NumberOfCoils);
            m_SignalGen.m_ReversePhase = ReadVal<bool>(v1,"reversephase", m_SignalGen.m_ReversePhase);
            m_SignalGen.m_PartialFourier = ReadVal<double>(v1,"partialfourier", m_SignalGen.m_PartialFourier);
            m_SignalGen.m_NoiseVariance = ReadVal<double>(v1,"noisevariance", m_SignalGen.m_NoiseVariance);
            m_SignalGen.m_tRep = ReadVal<double>(v1,"trep", m_SignalGen.m_tRep);
            m_SignalGen.m_SignalScale = ReadVal<double>(v1,"signalScale", m_SignalGen.m_SignalScale);
            m_SignalGen.m_tEcho = ReadVal<double>(v1,"tEcho", m_SignalGen.m_tEcho);
            m_SignalGen.m_tLine = ReadVal<double>(v1,"tLine", m_SignalGen.m_tLine);
            m_SignalGen.m_tInhom = ReadVal<double>(v1,"tInhom", m_SignalGen.m_tInhom);
            m_SignalGen.m_Bvalue = ReadVal<double>(v1,"bvalue", m_SignalGen.m_Bvalue);
            m_SignalGen.m_SimulateKspaceAcquisition = ReadVal<bool>(v1,"simulatekspace", m_SignalGen.m_SimulateKspaceAcquisition);

            m_SignalGen.m_AxonRadius = ReadVal<double>(v1,"axonRadius", m_SignalGen.m_AxonRadius);
            m_SignalGen.m_Spikes = ReadVal<unsigned int>(v1,"artifacts.spikesnum", m_SignalGen.m_Spikes);
            m_SignalGen.m_SpikeAmplitude = ReadVal<double>(v1,"artifacts.spikesscale", m_SignalGen.m_SpikeAmplitude);
            m_SignalGen.m_KspaceLineOffset = ReadVal<double>(v1,"artifacts.kspaceLineOffset", m_SignalGen.m_KspaceLineOffset);
            m_SignalGen.m_EddyStrength = ReadVal<double>(v1,"artifacts.eddyStrength", m_SignalGen.m_EddyStrength);
            m_SignalGen.m_Tau = ReadVal<double>(v1,"artifacts.eddyTau", m_SignalGen.m_Tau);
            m_SignalGen.m_CroppingFactor = ReadVal<double>(v1,"artifacts.aliasingfactor", m_SignalGen.m_CroppingFactor);
            m_SignalGen.m_DoAddGibbsRinging = ReadVal<bool>(v1,"artifacts.addringing", m_SignalGen.m_DoAddGibbsRinging);
            m_SignalGen.m_DoSimulateRelaxation = ReadVal<bool>(v1,"doSimulateRelaxation", m_SignalGen.m_DoSimulateRelaxation);
            m_SignalGen.m_DoDisablePartialVolume = ReadVal<bool>(v1,"doDisablePartialVolume", m_SignalGen.m_DoDisablePartialVolume);
            m_SignalGen.m_DoAddMotion = ReadVal<bool>(v1,"artifacts.doAddMotion", m_SignalGen.m_DoAddMotion);
            m_SignalGen.m_DoRandomizeMotion = ReadVal<bool>(v1,"artifacts.randomMotion", m_SignalGen.m_DoRandomizeMotion);
            m_SignalGen.m_Translation[0] = ReadVal<double>(v1,"artifacts.translation0", m_SignalGen.m_Translation[0]);
            m_SignalGen.m_Translation[1] = ReadVal<double>(v1,"artifacts.translation1", m_SignalGen.m_Translation[1]);
            m_SignalGen.m_Translation[2] = ReadVal<double>(v1,"artifacts.translation2", m_SignalGen.m_Translation[2]);
            m_SignalGen.m_Rotation[0] = ReadVal<double>(v1,"artifacts.rotation0", m_SignalGen.m_Rotation[0]);
            m_SignalGen.m_Rotation[1] = ReadVal<double>(v1,"artifacts.rotation1", m_SignalGen.m_Rotation[1]);
            m_SignalGen.m_Rotation[2] = ReadVal<double>(v1,"artifacts.rotation2", m_SignalGen.m_Rotation[2]);

            m_Misc.m_MotionVolumesBox = ReadVal<string>(v1,"artifacts.motionvolumes", m_Misc.m_MotionVolumesBox);
            m_SignalGen.m_MotionVolumes.clear();
            if (m_Misc.m_MotionVolumesBox=="random")
            {
                for (int i=0; i<m_SignalGen.GetNumVolumes(); i++)
                    m_SignalGen.m_MotionVolumes.push_back(rand()%2);
            }
            else if (!m_Misc.m_MotionVolumesBox.empty())
            {
                for (int i=0; i<m_SignalGen.GetNumVolumes(); i++)
                    m_SignalGen.m_MotionVolumes.push_back(false);

                stringstream stream(m_Misc.m_MotionVolumesBox);
                int n;
                while(stream >> n)
                {
                    if (n<m_SignalGen.GetNumVolumes() && n>=0)
                        m_SignalGen.m_MotionVolumes[n]=true;
                }
            }

            //            m_SignalGen.SetNumWeightedVolumes(ReadVal<unsigned int>(v1,"numgradients", m_SignalGen.GetNumWeightedVolumes()));
            SignalGenerationParameters::GradientListType gradients;
            BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second.get_child("gradients") )
            {
                SignalGenerationParameters::GradientType g;
                g[0] = ReadVal<double>(v2,"x",0);
                g[1] = ReadVal<double>(v2,"y",0);
                g[2] = ReadVal<double>(v2,"z",0);
                gradients.push_back(g);
            }
            m_SignalGen.SetGradienDirections(gradients);

            try
            {
                if (ReadVal<string>(v1,"artifacts.noisetype","")=="rice")
                {
                    m_NoiseModel = std::make_shared< mitk::RicianNoiseModel<ScalarType> >();
                    m_NoiseModel->SetNoiseVariance(ReadVal<double>(v1,"artifacts.noisevariance",m_NoiseModel->GetNoiseVariance()));
                }
            }
            catch(...) {}
            try
            {
                if (ReadVal<string>(v1,"artifacts.noisetype","")=="chisquare")
                {
                    m_NoiseModel = std::make_shared< mitk::ChiSquareNoiseModel<ScalarType> >();
                    m_NoiseModel->SetNoiseVariance(ReadVal<double>(v1,"artifacts.noisevariance",m_NoiseModel->GetNoiseVariance()));
                }
            }
            catch(...){ }

            BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second.get_child("compartments") )
            {
                mitk::DiffusionSignalModel<ScalarType>* signalModel = NULL;

                std::string model = ReadVal<std::string>(v2,"model","",true);
                if (model=="stick")
                {
                    mitk::StickModel<ScalarType>* model = new mitk::StickModel<ScalarType>();
                    model->SetDiffusivity(ReadVal<double>(v2,"d",model->GetDiffusivity()));
                    model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
                    model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
                    model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
                    if (ReadVal<string>(v2,"type","",true)=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (ReadVal<string>(v2,"type","",true)=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                else if (model=="tensor")
                {
                    mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
                    model->SetDiffusivity1(ReadVal<double>(v2,"d1",model->GetDiffusivity1()));
                    model->SetDiffusivity2(ReadVal<double>(v2,"d2",model->GetDiffusivity2()));
                    model->SetDiffusivity3(ReadVal<double>(v2,"d3",model->GetDiffusivity3()));
                    model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
                    model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
                    model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
                    if (ReadVal<string>(v2,"type","",true)=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (ReadVal<string>(v2,"type","",true)=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                else if (model=="ball")
                {
                    mitk::BallModel<ScalarType>* model = new mitk::BallModel<ScalarType>();
                    model->SetDiffusivity(ReadVal<double>(v2,"d",model->GetDiffusivity()));
                    model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
                    model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
                    model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
                    if (ReadVal<string>(v2,"type","",true)=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (ReadVal<string>(v2,"type","",true)=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                else if (model=="astrosticks")
                {
                    mitk::AstroStickModel<ScalarType>* model = new AstroStickModel<ScalarType>();
                    model->SetDiffusivity(ReadVal<double>(v2,"d",model->GetDiffusivity()));
                    model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
                    model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
                    model->SetRandomizeSticks(ReadVal<bool>(v2,"randomize",model->GetRandomizeSticks()));
                    model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
                    if (ReadVal<string>(v2,"type","",true)=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (ReadVal<string>(v2,"type","",true)=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                else if (model=="dot")
                {
                    mitk::DotModel<ScalarType>* model = new mitk::DotModel<ScalarType>();
                    model->SetT2(ReadVal<double>(v2,"t2",model->GetT2()));
                    model->SetT1(ReadVal<double>(v2,"t1",model->GetT1()));
                    model->m_CompartmentId = ReadVal<unsigned int>(v2,"ID",0,true);
                    if (ReadVal<string>(v2,"type","",true)=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (ReadVal<string>(v2,"type","",true)=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                else if (model=="prototype")
                {
                    mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
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
                            coeffs[k] = ReadVal<double>(v2,"kernels."+boost::lexical_cast<string>(j)+".coeffs."+boost::lexical_cast<string>(k),0,true);
                        }
                        model->SetShCoefficients( coeffs, ReadVal<double>(v2,"kernels."+boost::lexical_cast<string>(j)+".B0",0,true) );
                    }

                    if (ReadVal<string>(v2,"type","",true)=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (ReadVal<string>(v2,"type","",true)=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }

                if (signalModel!=NULL)
                {
                    signalModel->SetGradientList(gradients);
                    try{
                        itk::ImageFileReader<ItkDoubleImgType>::Pointer reader = itk::ImageFileReader<ItkDoubleImgType>::New();
                        reader->SetFileName(filename+"_VOLUME"+ReadVal<string>(v2,"ID","")+".nrrd");
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
    }

    try{
        itk::ImageFileReader<ItkDoubleImgType>::Pointer reader = itk::ImageFileReader<ItkDoubleImgType>::New();
        reader->SetFileName(filename+"_FMAP.nrrd");
        reader->Update();
        m_SignalGen.m_FrequencyMap = reader->GetOutput();
        MITK_INFO << "Frequency map loaded.";
    }
    catch(...)
    {
        MITK_INFO << "No frequency map found.";
    }

    try{
        itk::ImageFileReader<ItkUcharImgType>::Pointer reader = itk::ImageFileReader<ItkUcharImgType>::New();
        reader->SetFileName(filename+"_MASK.nrrd");
        reader->Update();
        m_SignalGen.m_MaskImage = reader->GetOutput();
        MITK_INFO << "Mask image loaded.";
    }
    catch(...)
    {
        MITK_INFO << "No mask image found.";
    }

    setlocale(LC_ALL, currLocale.c_str());
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::PrintSelf()
{
    MITK_INFO << "Not implemented :(";
}
