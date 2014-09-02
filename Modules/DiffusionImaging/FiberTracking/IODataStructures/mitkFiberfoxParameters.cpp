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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
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

typename mitk::SignalGenerationParameters::GradientListType mitk::SignalGenerationParameters::GetGradientDirections()
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

void mitk::SignalGenerationParameters::SetGradienDirections(mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer gradientList)
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
    parameters.put("fiberfox.image.signalScale", m_SignalGen.m_SignalScale);
    parameters.put("fiberfox.image.tEcho", m_SignalGen.m_tEcho);
    parameters.put("fiberfox.image.tLine", m_SignalGen.m_tLine);
    parameters.put("fiberfox.image.tInhom", m_SignalGen.m_tInhom);
    parameters.put("fiberfox.image.bvalue", m_SignalGen.m_Bvalue);
    parameters.put("fiberfox.image.simulatekspace", m_SignalGen.m_SimulateKspaceAcquisition);
    parameters.put("fiberfox.image.axonRadius", m_SignalGen.m_AxonRadius);
    parameters.put("fiberfox.image.diffusiondirectionmode", m_SignalGen.m_DiffusionDirectionMode);
    parameters.put("fiberfox.image.fiberseparationthreshold", m_SignalGen.m_FiberSeparationThreshold);
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
        if (dynamic_cast<mitk::RicianNoiseModel<ScalarType>*>(m_NoiseModel))
            parameters.put("fiberfox.image.artifacts.noisetype", "rice");
        else if (dynamic_cast<mitk::ChiSquareNoiseModel<ScalarType>*>(m_NoiseModel))
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
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".stick.d", model->GetDiffusivity());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".stick.t2", model->GetT2());
        }
        else  if (dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel))
        {
            mitk::TensorModel<ScalarType>* model = dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".tensor.d1", model->GetDiffusivity1());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".tensor.d2", model->GetDiffusivity2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".tensor.d3", model->GetDiffusivity3());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".tensor.t2", model->GetT2());
        }
        else  if (dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel))
        {
            mitk::RawShModel<ScalarType>* model = dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".prototype.minFA", model->GetFaRange().first);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".prototype.maxFA", model->GetFaRange().second);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".prototype.minADC", model->GetAdcRange().first);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".prototype.maxADC", model->GetAdcRange().second);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".prototype.numSamples", model->GetMaxNumKernels());
        }
        else  if (dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel))
        {
            mitk::BallModel<ScalarType>* model = dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".ball.d", model->GetDiffusivity());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".ball.t2", model->GetT2());
        }
        else  if (dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel))
        {
            mitk::AstroStickModel<ScalarType>* model = dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".astrosticks.d", model->GetDiffusivity());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".astrosticks.t2", model->GetT2());
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".astrosticks.randomize", model->GetRandomizeSticks());
        }
        else  if (dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel))
        {
            mitk::DotModel<ScalarType>* model = dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel);
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".dot.t2", model->GetT2());
        }

        if (signalModel!=NULL)
        {
            parameters.put("fiberfox.image.compartments."+boost::lexical_cast<string>(i)+".ID", signalModel->m_CompartmentId);

//            if (signalModel->GetVolumeFractionImage().IsNotNull())
//            {
//                try{
//                    itk::ImageFileWriter<ItkDoubleImgType>::Pointer writer = itk::ImageFileWriter<ItkDoubleImgType>::New();
//                    writer->SetFileName(filename+"_VOLUME"+boost::lexical_cast<string>(signalModel->m_CompartmentId)+".nrrd");
//                    writer->SetInput(signalModel->GetVolumeFractionImage());
//                    writer->Update();
//                    MITK_INFO << "Volume fraction image for compartment "+boost::lexical_cast<string>(signalModel->m_CompartmentId)+" saved.";
//                }
//                catch(...)
//                {
//                }
//            }
        }
    }

    boost::property_tree::xml_writer_settings<char> writerSettings(' ', 2);
    boost::property_tree::xml_parser::write_xml(filename, parameters, std::locale(), writerSettings);

//    if (m_SignalGen.m_DoAddDistortions)
//    {
//        try{
//            itk::ImageFileWriter<ItkDoubleImgType>::Pointer writer = itk::ImageFileWriter<ItkDoubleImgType>::New();
//            writer->SetFileName(filename+"_FMAP.nrrd");
//            writer->SetInput(m_SignalGen.m_FrequencyMap);
//            writer->Update();
//        }
//        catch(...)
//        {
//            MITK_INFO << "No frequency map saved.";
//        }
//    }

//    try{
//        itk::ImageFileWriter<ItkUcharImgType>::Pointer writer = itk::ImageFileWriter<ItkUcharImgType>::New();
//        writer->SetFileName(filename+"_MASK.nrrd");
//        writer->SetInput(m_SignalGen.m_MaskImage);
//        writer->Update();
//    }
//    catch(...)
//    {
//        MITK_INFO << "No mask image saved.";
//    }
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::LoadParameters(string filename)
{
    if(filename.empty())
        return;

    boost::property_tree::ptree parameterTree;
    boost::property_tree::xml_parser::read_xml(filename, parameterTree);

    m_FiberModelList.clear();
    m_NonFiberModelList.clear();
    if (m_NoiseModel!=NULL)
        delete m_NoiseModel;

    BOOST_FOREACH( boost::property_tree::ptree::value_type const& v1, parameterTree.get_child("fiberfox") )
    {
        if( v1.first == "fibers" )
        {
            m_Misc.m_CheckRealTimeFibersBox = v1.second.get<bool>("realtime", m_Misc.m_CheckRealTimeFibersBox);
            m_Misc.m_CheckAdvancedFiberOptionsBox = v1.second.get<bool>("showadvanced", m_Misc.m_CheckAdvancedFiberOptionsBox);
            m_Misc.m_CheckConstantRadiusBox = v1.second.get<bool>("constantradius", m_Misc.m_CheckConstantRadiusBox);
            m_Misc.m_CheckIncludeFiducialsBox = v1.second.get<bool>("includeFiducials", m_Misc.m_CheckIncludeFiducialsBox);

            switch (v1.second.get<unsigned int>("distribution", 0))
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
            m_FiberGen.m_Variance = v1.second.get<double>("variance", m_FiberGen.m_Variance);
            m_FiberGen.m_Density = v1.second.get<unsigned int>("density", m_FiberGen.m_Density);
            m_FiberGen.m_Sampling = v1.second.get<double>("spline.sampling", m_FiberGen.m_Sampling);
            m_FiberGen.m_Tension = v1.second.get<double>("spline.tension", m_FiberGen.m_Tension);
            m_FiberGen.m_Continuity = v1.second.get<double>("spline.continuity", m_FiberGen.m_Continuity);
            m_FiberGen.m_Bias = v1.second.get<double>("spline.bias", m_FiberGen.m_Bias);
            m_FiberGen.m_Rotation[0] = v1.second.get<double>("rotation.x", m_FiberGen.m_Rotation[0]);
            m_FiberGen.m_Rotation[1] = v1.second.get<double>("rotation.y", m_FiberGen.m_Rotation[1]);
            m_FiberGen.m_Rotation[2] = v1.second.get<double>("rotation.z", m_FiberGen.m_Rotation[2]);
            m_FiberGen.m_Translation[0] = v1.second.get<double>("translation.x", m_FiberGen.m_Translation[0]);
            m_FiberGen.m_Translation[1] = v1.second.get<double>("translation.y", m_FiberGen.m_Translation[1]);
            m_FiberGen.m_Translation[2] = v1.second.get<double>("translation.z", m_FiberGen.m_Translation[2]);
            m_FiberGen.m_Scale[0] = v1.second.get<double>("scale.x", m_FiberGen.m_Scale[0]);
            m_FiberGen.m_Scale[1] = v1.second.get<double>("scale.y", m_FiberGen.m_Scale[1]);
            m_FiberGen.m_Scale[2] = v1.second.get<double>("scale.z", m_FiberGen.m_Scale[2]);
        }
        else if ( v1.first == "image" )
        {
            m_Misc.m_SignalModelString = v1.second.get<string>("signalmodelstring", m_Misc.m_SignalModelString);
            m_Misc.m_ArtifactModelString = v1.second.get<string>("artifactmodelstring", m_Misc.m_ArtifactModelString);
            m_Misc.m_OutputPath = v1.second.get<string>("outpath", m_Misc.m_OutputPath);
            m_Misc.m_CheckOutputVolumeFractionsBox = v1.second.get<bool>("outputvolumefractions", m_Misc.m_CheckOutputVolumeFractionsBox);
            m_Misc.m_CheckAdvancedSignalOptionsBox = v1.second.get<bool>("showadvanced", m_Misc.m_CheckAdvancedSignalOptionsBox);
            m_Misc.m_CheckAddDistortionsBox = v1.second.get<bool>("artifacts.doAddDistortions", m_Misc.m_CheckAddDistortionsBox);
            m_Misc.m_CheckAddNoiseBox = v1.second.get<bool>("artifacts.addnoise", m_Misc.m_CheckAddNoiseBox);
            m_Misc.m_CheckAddGhostsBox = v1.second.get<bool>("artifacts.addghosts", m_Misc.m_CheckAddGhostsBox);
            m_Misc.m_CheckAddAliasingBox = v1.second.get<bool>("artifacts.addaliasing", m_Misc.m_CheckAddAliasingBox);
            m_Misc.m_CheckAddSpikesBox = v1.second.get<bool>("artifacts.addspikes", m_Misc.m_CheckAddSpikesBox);
            m_Misc.m_CheckAddEddyCurrentsBox = v1.second.get<bool>("artifacts.addeddycurrents", m_Misc.m_CheckAddEddyCurrentsBox);

            m_SignalGen.m_ImageRegion.SetSize(0, v1.second.get<int>("basic.size.x",m_SignalGen.m_ImageRegion.GetSize(0)));
            m_SignalGen.m_ImageRegion.SetSize(1, v1.second.get<int>("basic.size.y",m_SignalGen.m_ImageRegion.GetSize(1)));
            m_SignalGen.m_ImageRegion.SetSize(2, v1.second.get<int>("basic.size.z",m_SignalGen.m_ImageRegion.GetSize(2)));
            m_SignalGen.m_ImageSpacing[0] = v1.second.get<double>("basic.spacing.x",m_SignalGen.m_ImageSpacing[0]);
            m_SignalGen.m_ImageSpacing[1] = v1.second.get<double>("basic.spacing.y",m_SignalGen.m_ImageSpacing[1]);
            m_SignalGen.m_ImageSpacing[2] = v1.second.get<double>("basic.spacing.z",m_SignalGen.m_ImageSpacing[2]);
            m_SignalGen.m_ImageOrigin[0] = v1.second.get<double>("basic.origin.x",m_SignalGen.m_ImageOrigin[0]);
            m_SignalGen.m_ImageOrigin[1] = v1.second.get<double>("basic.origin.y",m_SignalGen.m_ImageOrigin[1]);
            m_SignalGen.m_ImageOrigin[2] = v1.second.get<double>("basic.origin.z",m_SignalGen.m_ImageOrigin[2]);
            m_SignalGen.m_ImageDirection[0][0] = v1.second.get<double>("basic.direction.1",m_SignalGen.m_ImageDirection[0][0]);
            m_SignalGen.m_ImageDirection[0][1] = v1.second.get<double>("basic.direction.2",m_SignalGen.m_ImageDirection[0][1]);
            m_SignalGen.m_ImageDirection[0][2] = v1.second.get<double>("basic.direction.3",m_SignalGen.m_ImageDirection[0][2]);
            m_SignalGen.m_ImageDirection[1][0] = v1.second.get<double>("basic.direction.4",m_SignalGen.m_ImageDirection[1][0]);
            m_SignalGen.m_ImageDirection[1][1] = v1.second.get<double>("basic.direction.5",m_SignalGen.m_ImageDirection[1][1]);
            m_SignalGen.m_ImageDirection[1][2] = v1.second.get<double>("basic.direction.6",m_SignalGen.m_ImageDirection[1][2]);
            m_SignalGen.m_ImageDirection[2][0] = v1.second.get<double>("basic.direction.7",m_SignalGen.m_ImageDirection[2][0]);
            m_SignalGen.m_ImageDirection[2][1] = v1.second.get<double>("basic.direction.8",m_SignalGen.m_ImageDirection[2][1]);
            m_SignalGen.m_ImageDirection[2][2] = v1.second.get<double>("basic.direction.9",m_SignalGen.m_ImageDirection[2][2]);

            m_SignalGen.m_SignalScale = v1.second.get<double>("signalScale", m_SignalGen.m_SignalScale);
            m_SignalGen.m_tEcho = v1.second.get<double>("tEcho", m_SignalGen.m_tEcho);
            m_SignalGen.m_tLine = v1.second.get<double>("tLine", m_SignalGen.m_tLine);
            m_SignalGen.m_tInhom = v1.second.get<double>("tInhom", m_SignalGen.m_tInhom);
            m_SignalGen.m_Bvalue = v1.second.get<double>("bvalue", m_SignalGen.m_Bvalue);
            m_SignalGen.m_SimulateKspaceAcquisition = v1.second.get<bool>("simulatekspace", m_SignalGen.m_SimulateKspaceAcquisition);

            m_SignalGen.m_AxonRadius = v1.second.get<double>("axonRadius", m_SignalGen.m_AxonRadius);
            switch (v1.second.get<int>("diffusiondirectionmode", 0))
            {
            case 0:
                m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS;
                break;
            case 1:
                m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::MAIN_FIBER_DIRECTIONS;
                break;
            case 2:
                m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::RANDOM_DIRECTIONS;
                break;
            default:
                m_SignalGen.m_DiffusionDirectionMode = SignalGenerationParameters::FIBER_TANGENT_DIRECTIONS;
            }
            m_SignalGen.m_FiberSeparationThreshold = v1.second.get<double>("fiberseparationthreshold", m_SignalGen.m_FiberSeparationThreshold);
            m_SignalGen.m_Spikes = v1.second.get<unsigned int>("artifacts.spikesnum", m_SignalGen.m_Spikes);
            m_SignalGen.m_SpikeAmplitude = v1.second.get<double>("artifacts.spikesscale", m_SignalGen.m_SpikeAmplitude);
            m_SignalGen.m_KspaceLineOffset = v1.second.get<double>("artifacts.kspaceLineOffset", m_SignalGen.m_KspaceLineOffset);
            m_SignalGen.m_EddyStrength = v1.second.get<double>("artifacts.eddyStrength", m_SignalGen.m_EddyStrength);
            m_SignalGen.m_Tau = v1.second.get<double>("artifacts.eddyTau", m_SignalGen.m_Tau);
            m_SignalGen.m_CroppingFactor = v1.second.get<double>("artifacts.aliasingfactor", m_SignalGen.m_CroppingFactor);
            m_SignalGen.m_DoAddGibbsRinging = v1.second.get<bool>("artifacts.addringing", m_SignalGen.m_DoAddGibbsRinging);
            m_SignalGen.m_DoSimulateRelaxation = v1.second.get<bool>("doSimulateRelaxation", m_SignalGen.m_DoSimulateRelaxation);
            m_SignalGen.m_DoDisablePartialVolume = v1.second.get<bool>("doDisablePartialVolume", m_SignalGen.m_DoDisablePartialVolume);
            m_SignalGen.m_DoAddMotion = v1.second.get<bool>("artifacts.doAddMotion", m_SignalGen.m_DoAddMotion);
            m_SignalGen.m_DoRandomizeMotion = v1.second.get<bool>("artifacts.randomMotion", m_SignalGen.m_DoRandomizeMotion);
            m_SignalGen.m_Translation[0] = v1.second.get<double>("artifacts.translation0", m_SignalGen.m_Translation[0]);
            m_SignalGen.m_Translation[1] = v1.second.get<double>("artifacts.translation1", m_SignalGen.m_Translation[1]);
            m_SignalGen.m_Translation[2] = v1.second.get<double>("artifacts.translation2", m_SignalGen.m_Translation[2]);
            m_SignalGen.m_Rotation[0] = v1.second.get<double>("artifacts.rotation0", m_SignalGen.m_Rotation[0]);
            m_SignalGen.m_Rotation[1] = v1.second.get<double>("artifacts.rotation1", m_SignalGen.m_Rotation[1]);
            m_SignalGen.m_Rotation[2] = v1.second.get<double>("artifacts.rotation2", m_SignalGen.m_Rotation[2]);
            m_SignalGen.SetNumWeightedVolumes(v1.second.get<unsigned int>("numgradients", m_SignalGen.GetNumWeightedVolumes()));

            try
            {
                if (v1.second.get<string>("artifacts.noisetype")=="rice")
                {
                    m_NoiseModel = new mitk::RicianNoiseModel<ScalarType>();
                    m_NoiseModel->SetNoiseVariance(v1.second.get<double>("artifacts.noisevariance"));
                }
            }
            catch(...) {}
            try
            {
                if (v1.second.get<string>("artifacts.noisetype")=="chisquare")
                {
                    m_NoiseModel = new mitk::ChiSquareNoiseModel<ScalarType>();
                    m_NoiseModel->SetNoiseVariance(v1.second.get<double>("artifacts.noisevariance"));
                }
            }
            catch(...){ }


            BOOST_FOREACH( boost::property_tree::ptree::value_type const& v2, v1.second.get_child("compartments") )
            {
                mitk::DiffusionSignalModel<ScalarType>* signalModel = NULL;
                try // stick model
                {
                    mitk::StickModel<ScalarType>* model = new mitk::StickModel<ScalarType>();
                    model->SetDiffusivity(v2.second.get<double>("stick.d"));
                    model->SetT2(v2.second.get<double>("stick.t2"));
                    model->m_CompartmentId = v2.second.get<unsigned int>("ID");
                    if (v2.second.get<string>("type")=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (v2.second.get<string>("type")=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                catch (...) { }

                try // tensor model
                {
                    mitk::TensorModel<ScalarType>* model = new mitk::TensorModel<ScalarType>();
                    model->SetDiffusivity1(v2.second.get<double>("tensor.d1"));
                    model->SetDiffusivity2(v2.second.get<double>("tensor.d2"));
                    model->SetDiffusivity3(v2.second.get<double>("tensor.d3"));
                    model->SetT2(v2.second.get<double>("tensor.t2"));
                    model->m_CompartmentId = v2.second.get<unsigned int>("ID");
                    if (v2.second.get<string>("type")=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (v2.second.get<string>("type")=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                catch (...) { }

                try // ball model
                {
                    mitk::BallModel<ScalarType>* model = new mitk::BallModel<ScalarType>();
                    model->SetDiffusivity(v2.second.get<double>("ball.d"));
                    model->SetT2(v2.second.get<double>("ball.t2"));
                    model->m_CompartmentId = v2.second.get<unsigned int>("ID");
                    if (v2.second.get<string>("type")=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (v2.second.get<string>("type")=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                catch (...) { }

                try // astrosticks model
                {
                    mitk::AstroStickModel<ScalarType>* model = new AstroStickModel<ScalarType>();
                    model->SetDiffusivity(v2.second.get<double>("astrosticks.d"));
                    model->SetT2(v2.second.get<double>("astrosticks.t2"));
                    model->SetRandomizeSticks(v2.second.get<bool>("astrosticks.randomize"));
                    model->m_CompartmentId = v2.second.get<unsigned int>("ID");
                    if (v2.second.get<string>("type")=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (v2.second.get<string>("type")=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                catch (...) { }

                try // dot model
                {
                    mitk::DotModel<ScalarType>* model = new mitk::DotModel<ScalarType>();
                    model->SetT2(v2.second.get<double>("dot.t2"));
                    model->m_CompartmentId = v2.second.get<unsigned int>("ID");
                    if (v2.second.get<string>("type")=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (v2.second.get<string>("type")=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                catch (...) { }

                try // prototype model
                {
                    mitk::RawShModel<ScalarType>* model = new mitk::RawShModel<ScalarType>();
                    model->SetMaxNumKernels(v2.second.get<unsigned int>("prototype.numSamples"));
                    model->SetFaRange(v2.second.get<double>("prototype.minFA"), v2.second.get<double>("prototype.maxFA"));
                    model->SetAdcRange(v2.second.get<double>("prototype.minADC"), v2.second.get<double>("prototype.maxADC"));
                    model->m_CompartmentId = v2.second.get<unsigned int>("ID");
                    if (v2.second.get<string>("type")=="fiber")
                        m_FiberModelList.push_back(model);
                    else if (v2.second.get<string>("type")=="non-fiber")
                        m_NonFiberModelList.push_back(model);
                    signalModel = model;
                }
                catch (...) { }

//                if (signalModel!=NULL)
//                {
//                    try{
//                        itk::ImageFileReader<ItkDoubleImgType>::Pointer reader = itk::ImageFileReader<ItkDoubleImgType>::New();
//                        reader->SetFileName(filename+"_VOLUME"+v2.second.get<string>("ID")+".nrrd");
//                        reader->Update();
//                        signalModel->SetVolumeFractionImage(reader->GetOutput());
//                    }
//                    catch(...)
//                    {
//                    }
//                }
            }
        }
    }

//    if (m_SignalGen.m_DoAddDistortions)
//    {
//        try{
//            itk::ImageFileReader<ItkDoubleImgType>::Pointer reader = itk::ImageFileReader<ItkDoubleImgType>::New();
//            reader->SetFileName(filename+"_FMAP.nrrd");
//            reader->Update();
//            m_SignalGen.m_FrequencyMap = reader->GetOutput();
//        }
//        catch(...)
//        {
//            MITK_INFO << "No frequency map saved.";
//        }
//    }

//    try{
//        itk::ImageFileReader<ItkUcharImgType>::Pointer reader = itk::ImageFileReader<ItkUcharImgType>::New();
//        reader->SetFileName(filename+"_MASK.nrrd");
//        reader->Update();
//        m_SignalGen.m_MaskImage = reader->GetOutput();
//    }
//    catch(...)
//    {
//        MITK_INFO << "No mask image saved.";
//    }
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::PrintSelf()
{
    MITK_INFO << "Not implemented :(";
}
