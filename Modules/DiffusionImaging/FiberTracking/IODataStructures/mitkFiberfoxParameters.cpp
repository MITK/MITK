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
#include "mitkFiberfoxParameters.h"

#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkAstroStickModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>

template< class ScalarType >
mitk::FiberfoxParameters< ScalarType >::FiberfoxParameters()
    : m_DoAddGibbsRinging(false)
    , m_ArtifactModelString("")
    , m_AxonRadius(0)
    , m_Bvalue(1000)
    , m_DoAddMotion(false)
    , m_DoDisablePartialVolume(false)
    , m_DoSimulateRelaxation(true)
    , m_EddyStrength(0)
    , m_Tau(70)
    , m_KspaceLineOffset(0)
    , m_NumGradients(6)
    , m_NumBaseline(1)
    , m_OutputPath("")
    , m_DoRandomizeMotion(true)
    , m_Repetitions(1)
    , m_SignalModelString("")
    , m_SignalScale(100)
    , m_SpikeAmplitude(1)
    , m_Spikes(0)
    , m_tEcho(100)
    , m_tInhom(50)
    , m_tLine(1)
    , m_CroppingFactor(1)
    , m_MaskImage(NULL)
    , m_FrequencyMap(NULL)
    , m_NoiseModel(NULL)
{
    m_ImageDirection.SetIdentity();
    m_ImageOrigin.Fill(0.0);
    m_ImageRegion.SetSize(0, 11);
    m_ImageRegion.SetSize(1, 11);
    m_ImageRegion.SetSize(2, 3);
    m_ImageSpacing.Fill(2.0);

    m_Translation.Fill(0.0);
    m_Rotation.Fill(0.0);

    m_ResultNode = mitk::DataNode::New();
    m_ParentNode = NULL;

    GenerateGradientHalfShell();
}

template< class ScalarType >
mitk::FiberfoxParameters< ScalarType >::~FiberfoxParameters()
{
    //    if (m_NoiseModel!=NULL)
    //        delete m_NoiseModel;
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::GenerateGradientHalfShell()
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
            // % (2*DIST_m_GradientDirections_PI);
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

template< class ScalarType >
std::vector< int > mitk::FiberfoxParameters< ScalarType >::GetBaselineIndices()
{
    std::vector< int > result;
    for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
        if (m_GradientDirections.at(i).GetNorm()<0.0001)
            result.push_back(i);
    return result;
}

template< class ScalarType >
unsigned int mitk::FiberfoxParameters< ScalarType >::GetFirstBaselineIndex()
{
    for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
        if (m_GradientDirections.at(i).GetNorm()<0.0001)
            return i;
    return -1;
}

template< class ScalarType >
bool mitk::FiberfoxParameters< ScalarType >::IsBaselineIndex(unsigned int idx)
{
    if (m_GradientDirections.size()>idx && m_GradientDirections.at(idx).GetNorm()<0.0001)
        return true;
    return false;
}

template< class ScalarType >
unsigned int mitk::FiberfoxParameters< ScalarType >::GetNumWeightedVolumes()
{
    return m_NumGradients;
}

template< class ScalarType >
unsigned int mitk::FiberfoxParameters< ScalarType >::GetNumBaselineVolumes()
{
    return m_NumBaseline;
}

template< class ScalarType >
unsigned int mitk::FiberfoxParameters< ScalarType >::GetNumVolumes()
{
    return m_GradientDirections.size();
}

template< class ScalarType >
typename mitk::FiberfoxParameters< ScalarType >::GradientListType mitk::FiberfoxParameters< ScalarType >::GetGradientDirections()
{
    return m_GradientDirections;
}

template< class ScalarType >
typename mitk::FiberfoxParameters< ScalarType >::GradientType mitk::FiberfoxParameters< ScalarType >::GetGradientDirection(unsigned int i)
{
    if (i<m_GradientDirections.size())
        return m_GradientDirections.at(i);
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::SetNumWeightedGradients(int numGradients)
{
    m_NumGradients = numGradients;
    GenerateGradientHalfShell();
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::SetGradienDirections(GradientListType gradientList)
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

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::SetGradienDirections(mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer gradientList)
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
void mitk::FiberfoxParameters< ScalarType >::LoadParameters(string filename)
{
    boost::property_tree::ptree parameters;
    boost::property_tree::xml_parser::read_xml(filename, parameters);

    m_FiberModelList.clear();
    m_NonFiberModelList.clear();
    if (m_NoiseModel!=NULL)
        delete m_NoiseModel;

    BOOST_FOREACH( boost::property_tree::ptree::value_type const& v1, parameters.get_child("fiberfox") )
    {
        if( v1.first == "image" )
        {
            m_ImageRegion.SetSize(0, v1.second.get<int>("basic.size.x"));
            m_ImageRegion.SetSize(1, v1.second.get<int>("basic.size.y"));
            m_ImageRegion.SetSize(2, v1.second.get<int>("basic.size.z"));
            m_ImageSpacing[0] = v1.second.get<double>("basic.spacing.x");
            m_ImageSpacing[1] = v1.second.get<double>("basic.spacing.y");
            m_ImageSpacing[2] = v1.second.get<double>("basic.spacing.z");
            m_NumGradients = v1.second.get<int>("basic.numgradients");
            GenerateGradientHalfShell();
            m_Bvalue = v1.second.get<int>("basic.bvalue");
            m_Repetitions = v1.second.get<int>("repetitions");
            m_SignalScale = v1.second.get<int>("signalScale");
            m_tEcho = v1.second.get<double>("tEcho");
            m_tLine = v1.second.get<double>("tLine");
            m_tInhom = v1.second.get<double>("tInhom");
            m_AxonRadius = v1.second.get<double>("axonRadius");
            m_DoSimulateRelaxation = v1.second.get<bool>("doSimulateRelaxation");
            m_DoDisablePartialVolume = v1.second.get<bool>("doDisablePartialVolume");
            if (v1.second.get<bool>("artifacts.addnoise"))
            {
                switch (v1.second.get<int>("artifacts.noisedistribution"))
                {
                case 0:
                    m_NoiseModel = new mitk::RicianNoiseModel< ScalarType >();
                    break;
                case 1:
                    m_NoiseModel = new mitk::ChiSquareNoiseModel< ScalarType >();
                    break;
                default:
                    m_NoiseModel = new mitk::RicianNoiseModel< ScalarType >();
                }
                m_NoiseModel->SetNoiseVariance(v1.second.get<double>("artifacts.noisevariance"));
            }

            m_KspaceLineOffset = v1.second.get<double>("artifacts.m_KspaceLineOffset");
            m_CroppingFactor = (100-v1.second.get<double>("artifacts.aliasingfactor"))/100;
            m_Spikes = v1.second.get<int>("artifacts.m_Spikesnum");
            m_SpikeAmplitude = v1.second.get<double>("artifacts.m_Spikesscale");
            m_EddyStrength = v1.second.get<double>("artifacts.m_EddyStrength");
            m_DoAddGibbsRinging = v1.second.get<bool>("artifacts.addringing");
            m_DoAddMotion = v1.second.get<bool>("artifacts.m_DoAddMotion");
            m_DoRandomizeMotion = v1.second.get<bool>("artifacts.m_RandomMotion");
            m_Translation[0] = v1.second.get<double>("artifacts.m_Translation0");
            m_Translation[1] = v1.second.get<double>("artifacts.m_Translation1");
            m_Translation[2] = v1.second.get<double>("artifacts.m_Translation2");
            m_Rotation[0] = v1.second.get<double>("artifacts.m_Rotation0");
            m_Rotation[1] = v1.second.get<double>("artifacts.m_Rotation1");
            m_Rotation[2] = v1.second.get<double>("artifacts.m_Rotation2");

            // compartment 1
            switch (v1.second.get<int>("compartment1.index"))
            {
            case 0:
                mitk::StickModel<double>* stickModel = new mitk::StickModel<double>();
                stickModel->SetGradientList(m_GradientDirections);
                stickModel->SetBvalue(m_Bvalue);
                stickModel->SetDiffusivity(v1.second.get<double>("compartment1.stick.d"));
                stickModel->SetT2(v1.second.get<double>("compartment1.stick.t2"));
                m_FiberModelList.push_back(stickModel);
                break;
            case 1:
                mitk::TensorModel<double>* zeppelinModel = new mitk::TensorModel<double>();
                zeppelinModel->SetGradientList(m_GradientDirections);
                zeppelinModel->SetBvalue(m_Bvalue);
                zeppelinModel->SetDiffusivity1(v1.second.get<double>("compartment1.zeppelin.d1"));
                zeppelinModel->SetDiffusivity2(v1.second.get<double>("compartment1.zeppelin.d2"));
                zeppelinModel->SetDiffusivity3(v1.second.get<double>("compartment1.zeppelin.d2"));
                zeppelinModel->SetT2(v1.second.get<double>("compartment1.zeppelin.t2"));
                m_FiberModelList.push_back(zeppelinModel);
                break;
            case 2:
                mitk::TensorModel<double>* tensorModel = new mitk::TensorModel<double>();
                tensorModel->SetGradientList(m_GradientDirections);
                tensorModel->SetBvalue(m_Bvalue);
                tensorModel->SetDiffusivity1(v1.second.get<double>("compartment1.tensor.d1"));
                tensorModel->SetDiffusivity2(v1.second.get<double>("compartment1.tensor.d2"));
                tensorModel->SetDiffusivity3(v1.second.get<double>("compartment1.tensor.d3"));
                tensorModel->SetT2(v1.second.get<double>("compartment1.tensor.t2"));
                m_FiberModelList.push_back(tensorModel);
                break;
            }

            // compartment 2
            switch (v1.second.get<int>("compartment2.index"))
            {
            case 0:
                mitk::StickModel<double>* stickModel = new mitk::StickModel<double>();
                stickModel->SetGradientList(m_GradientDirections);
                stickModel->SetBvalue(m_Bvalue);
                stickModel->SetDiffusivity(v1.second.get<double>("compartment2.stick.d"));
                stickModel->SetT2(v1.second.get<double>("compartment2.stick.t2"));
                m_FiberModelList.push_back(stickModel);
                break;
            case 1:
                mitk::TensorModel<double>* zeppelinModel = new mitk::TensorModel<double>();
                zeppelinModel->SetGradientList(m_GradientDirections);
                zeppelinModel->SetBvalue(m_Bvalue);
                zeppelinModel->SetDiffusivity1(v1.second.get<double>("compartment2.zeppelin.d1"));
                zeppelinModel->SetDiffusivity2(v1.second.get<double>("compartment2.zeppelin.d2"));
                zeppelinModel->SetDiffusivity3(v1.second.get<double>("compartment2.zeppelin.d2"));
                zeppelinModel->SetT2(v1.second.get<double>("compartment2.zeppelin.t2"));
                m_FiberModelList.push_back(zeppelinModel);
                break;
            case 2:
                mitk::TensorModel<double>* tensorModel = new mitk::TensorModel<double>();
                tensorModel->SetGradientList(m_GradientDirections);
                tensorModel->SetBvalue(m_Bvalue);
                tensorModel->SetDiffusivity1(v1.second.get<double>("compartment2.tensor.d1"));
                tensorModel->SetDiffusivity2(v1.second.get<double>("compartment2.tensor.d2"));
                tensorModel->SetDiffusivity3(v1.second.get<double>("compartment2.tensor.d3"));
                tensorModel->SetT2(v1.second.get<double>("compartment2.tensor.t2"));
                m_FiberModelList.push_back(tensorModel);
                break;
            }

            // compartment 3
            switch (v1.second.get<int>("compartment3.index"))
            {
            case 0:
                mitk::BallModel<double>* ballModel = new mitk::BallModel<double>();
                ballModel->SetGradientList(m_GradientDirections);
                ballModel->SetBvalue(m_Bvalue);
                ballModel->SetDiffusivity(v1.second.get<double>("compartment3.ball.d"));
                ballModel->SetT2(v1.second.get<double>("compartment3.ball.t2"));
                ballModel->SetWeight(v1.second.get<double>("compartment3.weight"));
                m_NonFiberModelList.push_back(ballModel);
                break;
            case 1:
                mitk::AstroStickModel<double>* astrosticksModel = new mitk::AstroStickModel<double>();
                astrosticksModel->SetGradientList(m_GradientDirections);
                astrosticksModel->SetBvalue(m_Bvalue);
                astrosticksModel->SetDiffusivity(v1.second.get<double>("compartment3.astrosticks.d"));
                astrosticksModel->SetT2(v1.second.get<double>("compartment3.astrosticks.t2"));
                astrosticksModel->SetRandomizeSticks(v1.second.get<bool>("compartment3.astrosticks.randomize"));
                astrosticksModel->SetWeight(v1.second.get<double>("compartment3.weight"));
                m_NonFiberModelList.push_back(astrosticksModel);
                break;
            case 2:
                mitk::DotModel<double>* dotModel = new mitk::DotModel<double>();
                dotModel->SetGradientList(m_GradientDirections);
                dotModel->SetT2(v1.second.get<double>("compartment3.dot.t2"));
                dotModel->SetWeight(v1.second.get<double>("compartment3.weight"));
                m_NonFiberModelList.push_back(dotModel);
                break;
            }

            // compartment 4
            switch (v1.second.get<int>("compartment4.index"))
            {
            case 0:
                mitk::BallModel<double>* ballModel = new mitk::BallModel<double>();
                ballModel->SetGradientList(m_GradientDirections);
                ballModel->SetBvalue(m_Bvalue);
                ballModel->SetDiffusivity(v1.second.get<double>("compartment4.ball.d"));
                ballModel->SetT2(v1.second.get<double>("compartment4.ball.t2"));
                ballModel->SetWeight(v1.second.get<double>("compartment4.weight"));
                m_NonFiberModelList.push_back(ballModel);
                break;
            case 1:
                mitk::AstroStickModel<double>* astrosticksModel = new mitk::AstroStickModel<double>();
                astrosticksModel->SetGradientList(m_GradientDirections);
                astrosticksModel->SetBvalue(m_Bvalue);
                astrosticksModel->SetDiffusivity(v1.second.get<double>("compartment4.astrosticks.d"));
                astrosticksModel->SetT2(v1.second.get<double>("compartment4.astrosticks.t2"));
                astrosticksModel->SetRandomizeSticks(v1.second.get<bool>("compartment4.astrosticks.randomize"));
                astrosticksModel->SetWeight(v1.second.get<double>("compartment4.weight"));
                m_NonFiberModelList.push_back(astrosticksModel);
                break;
            case 2:
                mitk::DotModel<double>* dotModel = new mitk::DotModel<double>();
                dotModel->SetGradientList(m_GradientDirections);
                dotModel->SetT2(v1.second.get<double>("compartment4.dot.t2"));
                dotModel->SetWeight(v1.second.get<double>("compartment4.weight"));
                m_NonFiberModelList.push_back(dotModel);
                break;
            }
        }
    }
}

template< class ScalarType >
void mitk::FiberfoxParameters< ScalarType >::PrintSelf()
{
    MITK_INFO << "m_ImageRegion: " << m_ImageRegion;
    MITK_INFO << "m_ImageSpacing: " << m_ImageSpacing;
    MITK_INFO << "m_ImageOrigin: " << m_ImageOrigin;
    MITK_INFO << "m_ImageDirection: " << m_ImageDirection;
    MITK_INFO << "m_NumGradients: " << m_NumGradients;
    MITK_INFO << "m_Bvalue: " << m_Bvalue;
    MITK_INFO << "m_Repetitions: " << m_Repetitions;
    MITK_INFO << "m_SignalScale: " << m_SignalScale;
    MITK_INFO << "m_tEcho: " << m_tEcho;
    MITK_INFO << "m_tLine: " << m_tLine;
    MITK_INFO << "m_tInhom: " << m_tInhom;
    MITK_INFO << "m_AxonRadius: " << m_AxonRadius;
    MITK_INFO << "m_KspaceLineOffset: " << m_KspaceLineOffset;
    MITK_INFO << "m_AddGibbsRinging: " << m_DoAddGibbsRinging;
    MITK_INFO << "m_EddyStrength: " << m_EddyStrength;
    MITK_INFO << "m_Spikes: " << m_Spikes;
    MITK_INFO << "m_SpikeAmplitude: " << m_SpikeAmplitude;
    MITK_INFO << "m_CroppingFactor: " << m_CroppingFactor;
    MITK_INFO << "m_DoSimulateRelaxation: " << m_DoSimulateRelaxation;
    MITK_INFO << "m_DoDisablePartialVolume: " << m_DoDisablePartialVolume;
    MITK_INFO << "m_DoAddMotion: " << m_DoAddMotion;
    MITK_INFO << "m_RandomMotion: " << m_DoRandomizeMotion;
    MITK_INFO << "m_Translation: " << m_Translation;
    MITK_INFO << "m_Rotation: " << m_Rotation;
    MITK_INFO << "m_SignalModelString: " << m_SignalModelString;
    MITK_INFO << "m_ArtifactModelString: " << m_ArtifactModelString;
    MITK_INFO << "m_OutputPath: " << m_OutputPath;
}
