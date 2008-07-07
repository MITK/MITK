/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkRigidRegistrationPreset.h"
#include "mitkStandardFileLocations.h"
#include "mitkMetricParameters.h"
#include "mitkOptimizerParameters.h"
#include "mitkTransformParameters.h"

namespace mitk {

  RigidRegistrationPreset::RigidRegistrationPreset()
  {
    m_Name = "";
    m_XmlFileName = "mitkRigidRegistrationPresets.xml";
  }

  RigidRegistrationPreset::~RigidRegistrationPreset()
  {
  }

  bool RigidRegistrationPreset::LoadPreset()
  {
    std::string location1 = MITK_ROOT;
    std::string location2 = "/../mbi-sb/QFunctionalities/QmitkRigidRegistration";
    std::string location = location1 + location2;
    mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch(location.c_str(), true);
    std::string xmlFileName = mitk::StandardFileLocations::GetInstance()->FindFile("mitkRigidRegistrationPresets.xml");

    if (!xmlFileName.empty())
    {
      m_XmlFileName = xmlFileName;
      return LoadPreset(m_XmlFileName);
    }
    else
      return false;
  }

  bool RigidRegistrationPreset::LoadPreset(std::string fileName)
  {
    if ( fileName.empty() )
      return false;

    vtkXMLParser::SetFileName( fileName.c_str() );

    if ( !vtkXMLParser::Parse() )
    {
  #ifdef INTERDEBUG
      std::cout<<"RigidRegistrationPreset::LoadPreset xml file cannot parse!"<<std::endl;
  #endif
    }

    return true;

  }

  void  RigidRegistrationPreset::StartElement (const char *elementName, const char **atts)
  {
    std::string elementNameString = elementName;
    if ( elementNameString == "preset" )
    {
      m_Name = ReadXMLStringAttribut( "NAME", atts );
    }
    else if (elementNameString == "transform")
    {
      itk::Array<double> transformValues;
      transformValues.SetSize(25);
      transformValues.fill(0);
      std::string transform = ReadXMLStringAttribut( "TRANSFORM", atts );
      double trans = atof(transform.c_str());
      transformValues[0] = trans;
      transformValues = this->loadTransformValues(transformValues, trans, atts);
      m_TransformValues[m_Name] = transformValues;
    }
    else if (elementNameString == "metric")
    {
      itk::Array<double> metricValues;
      metricValues.SetSize(25);
      metricValues.fill(0);
      std::string metric = ReadXMLStringAttribut( "METRIC", atts );
      double met = atof(metric.c_str());
      metricValues[0] = met;
      metricValues = this->loadMetricValues(metricValues, met, atts);
      m_MetricValues[m_Name] = metricValues;
    }
    else if (elementNameString == "optimizer")
    {
      itk::Array<double> optimizerValues;
      optimizerValues.SetSize(25);
      optimizerValues.fill(0);
      std::string optimizer = ReadXMLStringAttribut( "OPTIMIZER", atts );
      double opt = atof(optimizer.c_str());
      optimizerValues[0] = opt;
      optimizerValues = this->loadOptimizerValues(optimizerValues, opt, atts);
      m_OptimizerValues[m_Name] = optimizerValues;
    }
    else if (elementNameString == "interpolator")
    {
      itk::Array<double> interpolatorValues;
      interpolatorValues.SetSize(25);
      interpolatorValues.fill(0);
      std::string interpolator = ReadXMLStringAttribut( "INTERPOLATOR", atts );
      double inter = atof(interpolator.c_str());
      interpolatorValues[0] = inter;
      interpolatorValues = this->loadInterpolatorValues(interpolatorValues, inter, atts);
      m_InterpolatorValues[m_Name] = interpolatorValues;
    }
  }

  std::string RigidRegistrationPreset::ReadXMLStringAttribut( std::string name, const char** atts )
  {
    if(atts)
    {
      const char** attsIter = atts;

      while(*attsIter)
      {
        if ( name == *attsIter )
        {
          attsIter++;
          return *attsIter;
        }
        attsIter++;
        attsIter++;
      }
    }

    return std::string();
  }

  itk::Array<double> RigidRegistrationPreset::getTransformValues(std::string name)
  {
    return m_TransformValues[name];
  }

  itk::Array<double> RigidRegistrationPreset::getMetricValues(std::string name)
  {
    return m_MetricValues[name];
  }

  itk::Array<double> RigidRegistrationPreset::getOptimizerValues(std::string name)
  {
    return m_OptimizerValues[name];
  }

  itk::Array<double> RigidRegistrationPreset::getInterpolatorValues(std::string name)
  {
    return m_InterpolatorValues[name];
  }

  std::map<std::string, itk::Array<double> >& RigidRegistrationPreset::getTransformValuesPresets()
  {
    return m_TransformValues;
  }

  std::map<std::string, itk::Array<double> >& RigidRegistrationPreset::getMetricValuesPresets()
  {
    return m_MetricValues;
  }

  std::map<std::string, itk::Array<double> >& RigidRegistrationPreset::getOptimizerValuesPresets()
  {
    return m_OptimizerValues;
  }

  std::map<std::string, itk::Array<double> >& RigidRegistrationPreset::getInterpolatorValuesPresets()
  {
    return m_InterpolatorValues;
  }

  void RigidRegistrationPreset::save()
  {
    XMLWriter writer(m_XmlFileName.c_str());
    saveXML(writer);
  }

  void RigidRegistrationPreset::saveXML(mitk::XMLWriter& xmlWriter)
  {
    xmlWriter.BeginNode("mitkRigidRegistrationPresets");
    for( std::map<std::string, itk::Array<double> >::iterator iter = m_TransformValues.begin(); iter != m_TransformValues.end(); iter++ ) {
      std::string item = ((*iter).first.c_str());
      xmlWriter.BeginNode("preset");
      xmlWriter.WriteProperty("NAME", item);
      xmlWriter.BeginNode("transform");
      this->saveTransformValues(xmlWriter, item);
      xmlWriter.EndNode();
      xmlWriter.BeginNode("metric");
      this->saveMetricValues(xmlWriter, item);
      xmlWriter.EndNode();
      xmlWriter.BeginNode("optimizer");
      this->saveOptimizerValues(xmlWriter, item);
      xmlWriter.EndNode();
      xmlWriter.BeginNode("interpolator");
      this->saveInterpolatorValues(xmlWriter, item);
      xmlWriter.EndNode();
      xmlWriter.EndNode();
    }
    xmlWriter.EndNode();
  }

  void RigidRegistrationPreset::newPresets(std::map<std::string, itk::Array<double> > newTransformValues, std::map<std::string, itk::Array<double> > newMetricValues,
                                            std::map<std::string, itk::Array<double> > newOptimizerValues, std::map<std::string, itk::Array<double> > newInterpolatorValues)
  {
    m_TransformValues = newTransformValues;
    m_MetricValues = newMetricValues;
    m_OptimizerValues = newOptimizerValues;
    m_InterpolatorValues = newInterpolatorValues;
    save();
  }

  void RigidRegistrationPreset::saveTransformValues(mitk::XMLWriter& xmlWriter, std::string item)
  {
    itk::Array<double> transformValues = m_TransformValues[item];
    double transform = transformValues[0];
    xmlWriter.WriteProperty("TRANSFORM", transformValues[0]);
    if (transform == mitk::TransformParameters::TRANSLATIONTRANSFORM || transform == mitk::TransformParameters::SCALETRANSFORM || 
          transform == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM || transform == mitk::TransformParameters::VERSORTRANSFORM ||
          transform == mitk::TransformParameters::RIGID2DTRANSFORM || transform == mitk::TransformParameters::EULER2DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
    }
    else if (transform == mitk::TransformParameters::AFFINETRANSFORM || transform == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("SCALE6", transformValues[7]);
      xmlWriter.WriteProperty("SCALE7", transformValues[8]);
      xmlWriter.WriteProperty("SCALE8", transformValues[9]);
      xmlWriter.WriteProperty("SCALE9", transformValues[10]);
      xmlWriter.WriteProperty("SCALE10", transformValues[11]);
      xmlWriter.WriteProperty("SCALE11", transformValues[12]);
      xmlWriter.WriteProperty("SCALE12", transformValues[13]);
      xmlWriter.WriteProperty("SCALE13", transformValues[14]);
      xmlWriter.WriteProperty("SCALE14", transformValues[15]);
      xmlWriter.WriteProperty("SCALE15", transformValues[16]);
      xmlWriter.WriteProperty("SCALE16", transformValues[17]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[18]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[19]);
    }
    else if (transform == mitk::TransformParameters::RIGID3DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("SCALE6", transformValues[7]);
      xmlWriter.WriteProperty("SCALE7", transformValues[8]);
      xmlWriter.WriteProperty("SCALE8", transformValues[9]);
      xmlWriter.WriteProperty("SCALE9", transformValues[10]);
      xmlWriter.WriteProperty("SCALE10", transformValues[11]);
      xmlWriter.WriteProperty("SCALE11", transformValues[12]);
      xmlWriter.WriteProperty("SCALE12", transformValues[13]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[14]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[15]);
    }
    else if (transform == mitk::TransformParameters::EULER3DTRANSFORM || transform == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM
          || transform == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("SCALE6", transformValues[7]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[8]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[9]);
    }
    else if (transform == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM || transform == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("SCALE6", transformValues[7]);
      xmlWriter.WriteProperty("SCALE7", transformValues[8]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[9]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[10]);
    }
    else if (transform == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("SCALE6", transformValues[7]);
      xmlWriter.WriteProperty("SCALE7", transformValues[8]);
      xmlWriter.WriteProperty("SCALE8", transformValues[9]);
      xmlWriter.WriteProperty("SCALE9", transformValues[10]);
      xmlWriter.WriteProperty("SCALE10", transformValues[11]);
      xmlWriter.WriteProperty("SCALE11", transformValues[12]);
      xmlWriter.WriteProperty("SCALE12", transformValues[13]);
      xmlWriter.WriteProperty("SCALE13", transformValues[14]);
      xmlWriter.WriteProperty("SCALE14", transformValues[15]);
      xmlWriter.WriteProperty("SCALE15", transformValues[16]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[17]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[18]);
    }
    else if (transform == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("ANGLE", transformValues[7]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[8]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[9]);
    }
    else if (transform == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE", transformValues[6]);
      xmlWriter.WriteProperty("ANGLE", transformValues[7]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[8]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[9]);
    }
    else if (transform == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      xmlWriter.WriteProperty("USESCALES", transformValues[1]);
      xmlWriter.WriteProperty("SCALE1", transformValues[2]);
      xmlWriter.WriteProperty("SCALE2", transformValues[3]);
      xmlWriter.WriteProperty("SCALE3", transformValues[4]);
      xmlWriter.WriteProperty("SCALE4", transformValues[5]);
      xmlWriter.WriteProperty("SCALE5", transformValues[6]);
      xmlWriter.WriteProperty("SCALE6", transformValues[7]);
      xmlWriter.WriteProperty("SCALE", transformValues[8]);
      xmlWriter.WriteProperty("ANGLE", transformValues[9]);
      xmlWriter.WriteProperty("USEINITIALIZER", transformValues[10]);
      xmlWriter.WriteProperty("USEMOMENTS", transformValues[11]);
    }
  }

  void RigidRegistrationPreset::saveMetricValues(mitk::XMLWriter& xmlWriter, std::string item)
  {
    itk::Array<double> metricValues = m_MetricValues[item];
    double metric = metricValues[0];
    xmlWriter.WriteProperty("METRIC", metricValues[0]);
    xmlWriter.WriteProperty("COMPUTEGRADIENT", metricValues[1]);
    if (metric == mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC || metric == mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC
        || metric == mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC || metric == mitk::MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC
        || metric == mitk::MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
    {
    }
    else if (metric == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      xmlWriter.WriteProperty("HISTOGRAMBINS", metricValues[2]);
    }
    else if (metric == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      xmlWriter.WriteProperty("USESAMPLING", metricValues[2]);
      xmlWriter.WriteProperty("SPATIALSAMPLES", metricValues[3]);
      xmlWriter.WriteProperty("HISTOGRAMBINS", metricValues[4]);
    }
    else if (metric == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      xmlWriter.WriteProperty("LAMBDA", metricValues[2]);
    }
    else if (metric == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      xmlWriter.WriteProperty("SPATIALSAMPLES", metricValues[2]);
      xmlWriter.WriteProperty("FIXEDSTANDARDDEVIATION", metricValues[3]);
      xmlWriter.WriteProperty("MOVINGSTANDARDDEVIATION", metricValues[4]);
      xmlWriter.WriteProperty("USENORMALIZERANDSMOOTHER", metricValues[5]);
      xmlWriter.WriteProperty("FIXEDSMOOTHERVARIANCE", metricValues[6]);
      xmlWriter.WriteProperty("MOVINGSMOOTHERVARIANCE", metricValues[7]);
    }
  }

  void RigidRegistrationPreset::saveOptimizerValues(mitk::XMLWriter& xmlWriter, std::string item)
  {
    itk::Array<double> optimizerValues = m_OptimizerValues[item];
    double optimizer = optimizerValues[0];
    xmlWriter.WriteProperty("OPTIMIZER", optimizerValues[0]);
    xmlWriter.WriteProperty("MAXIMIZE", optimizerValues[1]);
    if (optimizer == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
    {
      xmlWriter.WriteProperty("STEPLENGTH", optimizerValues[2]);
      xmlWriter.WriteProperty("NUMBEROFSTEPS", optimizerValues[3]);
    }
    else if (optimizer == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER
            || optimizer == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
    {
      xmlWriter.WriteProperty("LEARNINGRATE", optimizerValues[2]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[3]);
    }
    else if (optimizer == mitk::OptimizerParameters::LBFGSBOPTIMIZER)
    {
    }
    else if (optimizer == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
    {
      xmlWriter.WriteProperty("SHRINKFACTOR", optimizerValues[2]);
      xmlWriter.WriteProperty("GROWTHFACTOR", optimizerValues[3]);
      xmlWriter.WriteProperty("EPSILON", optimizerValues[4]);
      xmlWriter.WriteProperty("INITIALRADIUS", optimizerValues[5]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[6]);
    }
    else if (optimizer == mitk::OptimizerParameters::POWELLOPTIMIZER)
    {
      xmlWriter.WriteProperty("STEPLENGTH", optimizerValues[2]);
      xmlWriter.WriteProperty("STEPTOLERANCE", optimizerValues[3]);
      xmlWriter.WriteProperty("VALUETOLERANCE", optimizerValues[4]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[5]);
    }
    else if (optimizer == mitk::OptimizerParameters::FRPROPTIMIZER)
    {
      xmlWriter.WriteProperty("USEFLETCHREEVES", optimizerValues[2]);
      xmlWriter.WriteProperty("STEPLENGTH", optimizerValues[3]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[4]);
    }
    else if (optimizer == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
    {
      xmlWriter.WriteProperty("GRADIENTMAGNITUDETOLERANCE", optimizerValues[2]);
      xmlWriter.WriteProperty("MINSTEPLENGTH", optimizerValues[3]);
      xmlWriter.WriteProperty("MAXSTEPLENGTH", optimizerValues[4]);
      xmlWriter.WriteProperty("RELAXATIONFACTOR", optimizerValues[5]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[6]);
    }
    else if (optimizer == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER || optimizer == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
    {
      xmlWriter.WriteProperty("GRADIENTMAGNITUDETOLERANCE", optimizerValues[2]);
      xmlWriter.WriteProperty("MINSTEPLENGTH", optimizerValues[3]);
      xmlWriter.WriteProperty("MAXSTEPLENGTH", optimizerValues[4]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[5]);
    }
    else if (optimizer == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
    {
      xmlWriter.WriteProperty("SIMPLEXDELTA1", optimizerValues[2]);
      xmlWriter.WriteProperty("SIMPLEXDELTA2", optimizerValues[3]);
      xmlWriter.WriteProperty("SIMPLEXDELTA3", optimizerValues[4]);
      xmlWriter.WriteProperty("SIMPLEXDELTA4", optimizerValues[5]);
      xmlWriter.WriteProperty("SIMPLEXDELTA5", optimizerValues[6]);
      xmlWriter.WriteProperty("SIMPLEXDELTA6", optimizerValues[7]);
      xmlWriter.WriteProperty("SIMPLEXDELTA7", optimizerValues[8]);
      xmlWriter.WriteProperty("SIMPLEXDELTA8", optimizerValues[9]);
      xmlWriter.WriteProperty("SIMPLEXDELTA9", optimizerValues[10]);
      xmlWriter.WriteProperty("SIMPLEXDELTA10", optimizerValues[11]);
      xmlWriter.WriteProperty("SIMPLEXDELTA11", optimizerValues[12]);
      xmlWriter.WriteProperty("SIMPLEXDELTA12", optimizerValues[13]);
      xmlWriter.WriteProperty("SIMPLEXDELTA13", optimizerValues[14]);
      xmlWriter.WriteProperty("SIMPLEXDELTA14", optimizerValues[15]);
      xmlWriter.WriteProperty("SIMPLEXDELTA15", optimizerValues[16]);
      xmlWriter.WriteProperty("SIMPLEXDELTA16", optimizerValues[17]);
      xmlWriter.WriteProperty("PARAMETERSCONVERGENCETOLERANCE", optimizerValues[18]);
      xmlWriter.WriteProperty("FUNCTIONCONVERGENCETOLERANCE", optimizerValues[19]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[20]);
    }
    else if (optimizer == mitk::OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
    {
    }
    else if (optimizer == mitk::OptimizerParameters::LBFGSOPTIMIZER)
    {
      xmlWriter.WriteProperty("GRADIENTCONVERGENCETOLERANCE", optimizerValues[2]);
      xmlWriter.WriteProperty("LINESEARCHACCURACY", optimizerValues[3]);
      xmlWriter.WriteProperty("DEFAULTSTEPLENGTH", optimizerValues[4]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[5]);
      xmlWriter.WriteProperty("USETRACE", optimizerValues[6]);
    }
    else if (optimizer == mitk::OptimizerParameters::SPSAOPTIMIZER)
    {
      xmlWriter.WriteProperty("a", optimizerValues[2]);
      xmlWriter.WriteProperty("A", optimizerValues[3]);
      xmlWriter.WriteProperty("ALPHA", optimizerValues[4]);
      xmlWriter.WriteProperty("c", optimizerValues[5]);
      xmlWriter.WriteProperty("GAMMA", optimizerValues[6]);
      xmlWriter.WriteProperty("TOLERANCE", optimizerValues[7]);
      xmlWriter.WriteProperty("STATEOFCONVERGENCEDECAYRATE", optimizerValues[8]);
      xmlWriter.WriteProperty("MINNUMBERITERATIONS", optimizerValues[9]);
      xmlWriter.WriteProperty("NUMBERPERTURBATIONS", optimizerValues[10]);
      xmlWriter.WriteProperty("NUMBERITERATIONS", optimizerValues[11]);
    }
  }

  void RigidRegistrationPreset::saveInterpolatorValues(mitk::XMLWriter& xmlWriter, std::string item)
  {
    itk::Array<double> interpolatorValues = m_InterpolatorValues[item];
    xmlWriter.WriteProperty("INTERPOLATOR", interpolatorValues[0]);
  }

  itk::Array<double> RigidRegistrationPreset::loadTransformValues(itk::Array<double> transformValues, double transform, const char **atts)
  {
    if (transform == mitk::TransformParameters::TRANSLATIONTRANSFORM || transform == mitk::TransformParameters::SCALETRANSFORM || 
          transform == mitk::TransformParameters::SCALELOGARITHMICTRANSFORM || transform == mitk::TransformParameters::VERSORTRANSFORM ||
          transform == mitk::TransformParameters::RIGID2DTRANSFORM || transform == mitk::TransformParameters::EULER2DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
    }
    else if (transform == mitk::TransformParameters::AFFINETRANSFORM || transform == mitk::TransformParameters::FIXEDCENTEROFROTATIONAFFINETRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string scale6 = ReadXMLStringAttribut( "SCALE6", atts );
      double sca6 = atof(scale6.c_str());
      transformValues[7] = sca6;
      std::string scale7 = ReadXMLStringAttribut( "SCALE7", atts );
      double sca7 = atof(scale7.c_str());
      transformValues[8] = sca7;
      std::string scale8 = ReadXMLStringAttribut( "SCALE8", atts );
      double sca8 = atof(scale8.c_str());
      transformValues[9] = sca8;
      std::string scale9 = ReadXMLStringAttribut( "SCALE9", atts );
      double sca9 = atof(scale9.c_str());
      transformValues[10] = sca9;
      std::string scale10 = ReadXMLStringAttribut( "SCALE10", atts );
      double sca10 = atof(scale10.c_str());
      transformValues[11] = sca10;
      std::string scale11 = ReadXMLStringAttribut( "SCALE11", atts );
      double sca11 = atof(scale11.c_str());
      transformValues[12] = sca11;
      std::string scale12 = ReadXMLStringAttribut( "SCALE12", atts );
      double sca12 = atof(scale12.c_str());
      transformValues[13] = sca12;
      std::string scale13 = ReadXMLStringAttribut( "SCALE13", atts );
      double sca13 = atof(scale13.c_str());
      transformValues[14] = sca13;
      std::string scale14 = ReadXMLStringAttribut( "SCALE14", atts );
      double sca14 = atof(scale14.c_str());
      transformValues[15] = sca14;
      std::string scale15 = ReadXMLStringAttribut( "SCALE15", atts );
      double sca15 = atof(scale15.c_str());
      transformValues[16] = sca15;
      std::string scale16 = ReadXMLStringAttribut( "SCALE16", atts );
      double sca16 = atof(scale16.c_str());
      transformValues[17] = sca16;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[18] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[19] = useMo;
    }
    else if (transform == mitk::TransformParameters::RIGID3DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string scale6 = ReadXMLStringAttribut( "SCALE6", atts );
      double sca6 = atof(scale6.c_str());
      transformValues[7] = sca6;
      std::string scale7 = ReadXMLStringAttribut( "SCALE7", atts );
      double sca7 = atof(scale7.c_str());
      transformValues[8] = sca7;
      std::string scale8 = ReadXMLStringAttribut( "SCALE8", atts );
      double sca8 = atof(scale8.c_str());
      transformValues[9] = sca8;
      std::string scale9 = ReadXMLStringAttribut( "SCALE9", atts );
      double sca9 = atof(scale9.c_str());
      transformValues[10] = sca9;
      std::string scale10 = ReadXMLStringAttribut( "SCALE10", atts );
      double sca10 = atof(scale10.c_str());
      transformValues[11] = sca10;
      std::string scale11 = ReadXMLStringAttribut( "SCALE11", atts );
      double sca11 = atof(scale11.c_str());
      transformValues[12] = sca11;
      std::string scale12 = ReadXMLStringAttribut( "SCALE12", atts );
      double sca12 = atof(scale12.c_str());
      transformValues[13] = sca12;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[14] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[15] = useMo;
    }
    else if (transform == mitk::TransformParameters::EULER3DTRANSFORM || transform == mitk::TransformParameters::CENTEREDEULER3DTRANSFORM
          || transform == mitk::TransformParameters::VERSORRIGID3DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string scale6 = ReadXMLStringAttribut( "SCALE6", atts );
      double sca6 = atof(scale6.c_str());
      transformValues[7] = sca6;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[8] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[9] = useMo;
    }
    else if (transform == mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM || transform == mitk::TransformParameters::SIMILARITY3DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string scale6 = ReadXMLStringAttribut( "SCALE6", atts );
      double sca6 = atof(scale6.c_str());
      transformValues[7] = sca6;
      std::string scale7 = ReadXMLStringAttribut( "SCALE7", atts );
      double sca7 = atof(scale7.c_str());
      transformValues[8] = sca7;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[9] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[10] = useMo;
    }
    else if (transform == mitk::TransformParameters::SCALESKEWVERSOR3DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string scale6 = ReadXMLStringAttribut( "SCALE6", atts );
      double sca6 = atof(scale6.c_str());
      transformValues[7] = sca6;
      std::string scale7 = ReadXMLStringAttribut( "SCALE7", atts );
      double sca7 = atof(scale7.c_str());
      transformValues[8] = sca7;
      std::string scale8 = ReadXMLStringAttribut( "SCALE8", atts );
      double sca8 = atof(scale8.c_str());
      transformValues[9] = sca8;
      std::string scale9 = ReadXMLStringAttribut( "SCALE9", atts );
      double sca9 = atof(scale9.c_str());
      transformValues[10] = sca9;
      std::string scale10 = ReadXMLStringAttribut( "SCALE10", atts );
      double sca10 = atof(scale10.c_str());
      transformValues[11] = sca10;
      std::string scale11 = ReadXMLStringAttribut( "SCALE11", atts );
      double sca11 = atof(scale11.c_str());
      transformValues[12] = sca11;
      std::string scale12 = ReadXMLStringAttribut( "SCALE12", atts );
      double sca12 = atof(scale12.c_str());
      transformValues[13] = sca12;
      std::string scale13 = ReadXMLStringAttribut( "SCALE13", atts );
      double sca13 = atof(scale13.c_str());
      transformValues[14] = sca13;
      std::string scale14 = ReadXMLStringAttribut( "SCALE14", atts );
      double sca14 = atof(scale14.c_str());
      transformValues[15] = sca14;
      std::string scale15 = ReadXMLStringAttribut( "SCALE15", atts );
      double sca15 = atof(scale15.c_str());
      transformValues[16] = sca15;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[17] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[18] = useMo;
    }
    else if (transform == mitk::TransformParameters::CENTEREDRIGID2DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string angle = ReadXMLStringAttribut( "ANGLE", atts );
      double ang = atof(angle.c_str());
      transformValues[7] = ang;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[8] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[9] = useMo;
    }
    else if (transform == mitk::TransformParameters::SIMILARITY2DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale = ReadXMLStringAttribut( "SCALE", atts );
      double sca = atof(scale.c_str());
      transformValues[6] = sca;
      std::string angle = ReadXMLStringAttribut( "ANGLE", atts );
      double ang = atof(angle.c_str());
      transformValues[7] = ang;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[8] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[9] = useMo;
    }
    else if (transform == mitk::TransformParameters::CENTEREDSIMILARITY2DTRANSFORM)
    {
      std::string useScales = ReadXMLStringAttribut( "USESCALES", atts );
      double useSca = atof(useScales.c_str());
      transformValues[1] = useSca;
      std::string scale1 = ReadXMLStringAttribut( "SCALE1", atts );
      double sca1 = atof(scale1.c_str());
      transformValues[2] = sca1;
      std::string scale2 = ReadXMLStringAttribut( "SCALE2", atts );
      double sca2 = atof(scale2.c_str());
      transformValues[3] = sca2;
      std::string scale3 = ReadXMLStringAttribut( "SCALE3", atts );
      double sca3 = atof(scale3.c_str());
      transformValues[4] = sca3;
      std::string scale4 = ReadXMLStringAttribut( "SCALE4", atts );
      double sca4 = atof(scale4.c_str());
      transformValues[5] = sca4;
      std::string scale5 = ReadXMLStringAttribut( "SCALE5", atts );
      double sca5 = atof(scale5.c_str());
      transformValues[6] = sca5;
      std::string scale6 = ReadXMLStringAttribut( "SCALE6", atts );
      double sca6 = atof(scale6.c_str());
      transformValues[7] = sca6;
      std::string scale = ReadXMLStringAttribut( "SCALE", atts );
      double sca = atof(scale.c_str());
      transformValues[8] = sca;
      std::string angle = ReadXMLStringAttribut( "ANGLE", atts );
      double ang = atof(angle.c_str());
      transformValues[9] = ang;
      std::string useInitializer = ReadXMLStringAttribut( "USEINITIALIZER", atts );
      double useIni = atof(useInitializer.c_str());
      transformValues[10] = useIni;
      std::string useMoments = ReadXMLStringAttribut( "USEMOMENTS", atts );
      double useMo = atof(useMoments.c_str());
      transformValues[11] = useMo;
    }
    return transformValues;
  }

  itk::Array<double> RigidRegistrationPreset::loadMetricValues(itk::Array<double> metricValues, double metric, const char **atts)
  {
    std::string computeGradient = ReadXMLStringAttribut( "COMPUTEGRADIENT", atts );
    double compGra = atof(computeGradient.c_str());
    metricValues[1] = compGra;
    if (metric == mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC || metric == mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC
        || metric == mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC || metric == mitk::MetricParameters::MATCHCARDINALITYIMAGETOIMAGEMETRIC
        || metric == mitk::MetricParameters::KAPPASTATISTICIMAGETOIMAGEMETRIC)
    {
    }
    else if (metric == mitk::MetricParameters::KULLBACKLEIBLERCOMPAREHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::CORRELATIONCOEFFICIENTHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::MEANSQUARESHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC
      || metric == mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {
      std::string histogramBins = ReadXMLStringAttribut( "HISTOGRAMBINS", atts );
      double histBins = atof(histogramBins.c_str());
      metricValues[2] = histBins;
    }
    else if (metric == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      std::string useSampling = ReadXMLStringAttribut( "USESAMPLING", atts );
      double useSamp = atof(useSampling.c_str());
      metricValues[2] = useSamp;
      std::string spatialSamples = ReadXMLStringAttribut( "SPATIALSAMPLES", atts );
      double spatSamp = atof(spatialSamples.c_str());
      metricValues[3] = spatSamp;
      std::string histogramBins = ReadXMLStringAttribut( "HISTOGRAMBINS", atts );
      double histBins = atof(histogramBins.c_str());
      metricValues[4] = histBins;
    }
    else if (metric == mitk::MetricParameters::MEANRECIPROCALSQUAREDIFFERENCEIMAGETOIMAGEMETRIC)
    {
      std::string lambda = ReadXMLStringAttribut( "LAMBDA", atts );
      double lamb = atof(lambda.c_str());
      metricValues[2] = lamb;
    }
    else if (metric == mitk::MetricParameters::MUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      std::string spatialSamples = ReadXMLStringAttribut( "SPATIALSAMPLES", atts );
      double spatSamp = atof(spatialSamples.c_str());
      metricValues[2] = spatSamp;
      std::string fixedStandardDeviation = ReadXMLStringAttribut( "FIXEDSTANDARDDEVIATION", atts );
      double fiStaDev = atof(fixedStandardDeviation.c_str());
      metricValues[3] = fiStaDev;
      std::string movingStandardDeviation = ReadXMLStringAttribut( "MOVINGSTANDARDDEVIATION", atts );
      double moStaDev = atof(movingStandardDeviation.c_str());
      metricValues[4] = moStaDev;
      std::string useNormalizer = ReadXMLStringAttribut( "USENORMALIZERANDSMOOTHER", atts );
      double useNormal = atof(useNormalizer.c_str());
      metricValues[5] = useNormal;
      std::string fixedSmootherVariance = ReadXMLStringAttribut( "FIXEDSMOOTHERVARIANCE", atts );
      double fiSmoVa = atof(fixedSmootherVariance.c_str());
      metricValues[6] = fiSmoVa;
      std::string movingSmootherVariance = ReadXMLStringAttribut( "MOVINGSMOOTHERVARIANCE", atts );
      double moSmoVa = atof(movingSmootherVariance.c_str());
      metricValues[7] = moSmoVa;
    }
    return metricValues;
  }

  itk::Array<double> RigidRegistrationPreset::loadOptimizerValues(itk::Array<double> optimizerValues, double optimizer, const char **atts)
  {
    std::string maximize = ReadXMLStringAttribut( "MAXIMIZE", atts );
    double max = atof(maximize.c_str());
    optimizerValues[1] = max;
    if (optimizer == mitk::OptimizerParameters::EXHAUSTIVEOPTIMIZER)
    {
      std::string stepLength = ReadXMLStringAttribut( "STEPLENGTH", atts );
      double stepLe = atof(stepLength.c_str());
      optimizerValues[2] = stepLe;
      std::string numberOfSteps = ReadXMLStringAttribut( "NUMBEROFSTEPS", atts );
      double numSteps = atof(numberOfSteps.c_str());
      optimizerValues[3] = numSteps;
    }
    else if (optimizer == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER
            || optimizer == mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
    {
      std::string learningRate = ReadXMLStringAttribut( "LEARNINGRATE", atts );
      double learn = atof(learningRate.c_str());
      optimizerValues[2] = learn;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[3] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::LBFGSBOPTIMIZER)
    {
    }
    else if (optimizer == mitk::OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
    {
      std::string shrinkFactor = ReadXMLStringAttribut( "SHRINKFACTOR", atts );
      double shrink = atof(shrinkFactor.c_str());
      optimizerValues[2] = shrink;
      std::string growthFactor = ReadXMLStringAttribut( "GROWTHFACTOR", atts );
      double growth = atof(growthFactor.c_str());
      optimizerValues[3] = growth;
      std::string epsilon = ReadXMLStringAttribut( "EPSILON", atts );
      double eps = atof(epsilon.c_str());
      optimizerValues[4] = eps;
      std::string initialRadius = ReadXMLStringAttribut( "INITIALRADIUS", atts );
      double initRad = atof(initialRadius.c_str());
      optimizerValues[5] = initRad;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[6] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::POWELLOPTIMIZER)
    {
      std::string stepLength = ReadXMLStringAttribut( "STEPLENGTH", atts );
      double stepLe = atof(stepLength.c_str());
      optimizerValues[2] = stepLe;
      std::string stepTolerance = ReadXMLStringAttribut( "STEPTOLERANCE", atts );
      double stepTo = atof(stepTolerance.c_str());
      optimizerValues[3] = stepTo;
      std::string valueTolerance = ReadXMLStringAttribut( "VALUETOLERANCE", atts );
      double valTo = atof(valueTolerance.c_str());
      optimizerValues[4] = valTo;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[5] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::FRPROPTIMIZER)
    {
      std::string useFletchReeves = ReadXMLStringAttribut( "USEFLETCHREEVES", atts );
      double useFleRe = atof(useFletchReeves.c_str());
      optimizerValues[2] = useFleRe;
      std::string stepLength = ReadXMLStringAttribut( "STEPLENGTH", atts );
      double stepLe = atof(stepLength.c_str());
      optimizerValues[3] = stepLe;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[4] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
    {
      std::string gradientMagnitudeTolerance = ReadXMLStringAttribut( "GRADIENTMAGNITUDETOLERANCE", atts );
      double graMagTo = atof(gradientMagnitudeTolerance.c_str());
      optimizerValues[2] = graMagTo;
      std::string minStepLength = ReadXMLStringAttribut( "MINSTEPLENGTH", atts );
      double minStep = atof(minStepLength.c_str());
      optimizerValues[3] = minStep;
      std::string maxStepLength = ReadXMLStringAttribut( "MAXSTEPLENGTH", atts );
      double maxStep = atof(maxStepLength.c_str());
      optimizerValues[4] = maxStep;
      std::string relaxationFactor = ReadXMLStringAttribut( "RELAXATIONFACTOR", atts );
      double relFac = atof(relaxationFactor.c_str());
      optimizerValues[5] = relFac;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[6] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER || optimizer == mitk::OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
    {
      std::string gradientMagnitudeTolerance = ReadXMLStringAttribut( "GRADIENTMAGNITUDETOLERANCE", atts );
      double graMagTo = atof(gradientMagnitudeTolerance.c_str());
      optimizerValues[2] = graMagTo;
      std::string minStepLength = ReadXMLStringAttribut( "MINSTEPLENGTH", atts );
      double minStep = atof(minStepLength.c_str());
      optimizerValues[3] = minStep;
      std::string maxStepLength = ReadXMLStringAttribut( "MAXSTEPLENGTH", atts );
      double maxStep = atof(maxStepLength.c_str());
      optimizerValues[4] = maxStep;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[5] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::AMOEBAOPTIMIZER)
    {
      std::string simplexDelta1 = ReadXMLStringAttribut( "SIMPLEXDELTA1", atts );
      double simpDel1 = atof(simplexDelta1.c_str());
      optimizerValues[2] = simpDel1;
      std::string simplexDelta2 = ReadXMLStringAttribut( "SIMPLEXDELTA2", atts );
      double simpDel2 = atof(simplexDelta2.c_str());
      optimizerValues[3] = simpDel2;
      std::string simplexDelta3 = ReadXMLStringAttribut( "SIMPLEXDELTA3", atts );
      double simpDel3 = atof(simplexDelta3.c_str());
      optimizerValues[4] = simpDel3;
      std::string simplexDelta4 = ReadXMLStringAttribut( "SIMPLEXDELTA4", atts );
      double simpDel4 = atof(simplexDelta4.c_str());
      optimizerValues[5] = simpDel4;
      std::string simplexDelta5 = ReadXMLStringAttribut( "SIMPLEXDELTA5", atts );
      double simpDel5 = atof(simplexDelta5.c_str());
      optimizerValues[6] = simpDel5;
      std::string simplexDelta6 = ReadXMLStringAttribut( "SIMPLEXDELTA6", atts );
      double simpDel6 = atof(simplexDelta6.c_str());
      optimizerValues[7] = simpDel6;
      std::string simplexDelta7 = ReadXMLStringAttribut( "SIMPLEXDELTA7", atts );
      double simpDel7 = atof(simplexDelta7.c_str());
      optimizerValues[8] = simpDel7;
      std::string simplexDelta8 = ReadXMLStringAttribut( "SIMPLEXDELTA8", atts );
      double simpDel8 = atof(simplexDelta8.c_str());
      optimizerValues[9] = simpDel8;
      std::string simplexDelta9 = ReadXMLStringAttribut( "SIMPLEXDELTA9", atts );
      double simpDel9 = atof(simplexDelta9.c_str());
      optimizerValues[10] = simpDel9;
      std::string simplexDelta10 = ReadXMLStringAttribut( "SIMPLEXDELTA10", atts );
      double simpDel10 = atof(simplexDelta10.c_str());
      optimizerValues[11] = simpDel10;
      std::string simplexDelta11 = ReadXMLStringAttribut( "SIMPLEXDELTA11", atts );
      double simpDel11 = atof(simplexDelta11.c_str());
      optimizerValues[12] = simpDel11;
      std::string simplexDelta12 = ReadXMLStringAttribut( "SIMPLEXDELTA12", atts );
      double simpDel12 = atof(simplexDelta12.c_str());
      optimizerValues[13] = simpDel12;
      std::string simplexDelta13 = ReadXMLStringAttribut( "SIMPLEXDELTA13", atts );
      double simpDel13 = atof(simplexDelta13.c_str());
      optimizerValues[14] = simpDel13;
      std::string simplexDelta14 = ReadXMLStringAttribut( "SIMPLEXDELTA14", atts );
      double simpDel14 = atof(simplexDelta14.c_str());
      optimizerValues[15] = simpDel14;
      std::string simplexDelta15 = ReadXMLStringAttribut( "SIMPLEXDELTA15", atts );
      double simpDel15 = atof(simplexDelta15.c_str());
      optimizerValues[16] = simpDel15;
      std::string simplexDelta16 = ReadXMLStringAttribut( "SIMPLEXDELTA16", atts );
      double simpDel16 = atof(simplexDelta16.c_str());
      optimizerValues[17] = simpDel16;
      std::string parametersConvergenceTolerance = ReadXMLStringAttribut( "PARAMETERSCONVERGENCETOLERANCE", atts );
      double paramConv = atof(parametersConvergenceTolerance.c_str());
      optimizerValues[18] = paramConv;
      std::string functionConvergenceTolerance = ReadXMLStringAttribut( "FUNCTIONCONVERGENCETOLERANCE", atts );
      double funcConv = atof(functionConvergenceTolerance.c_str());
      optimizerValues[19] = funcConv;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[20] = numIt;
    }
    else if (optimizer == mitk::OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
    {
    }
    else if (optimizer == mitk::OptimizerParameters::LBFGSOPTIMIZER)
    {
      std::string GradientConvergenceTolerance = ReadXMLStringAttribut( "GRADIENTCONVERGENCETOLERANCE", atts );
      double graConTo = atof(GradientConvergenceTolerance.c_str());
      optimizerValues[2] = graConTo;
      std::string lineSearchAccuracy = ReadXMLStringAttribut( "LINESEARCHACCURACY", atts );
      double lineSearch = atof(lineSearchAccuracy.c_str());
      optimizerValues[3] = lineSearch;
      std::string defaultStepLength = ReadXMLStringAttribut( "DEFAULTSTEPLENGTH", atts );
      double defStep = atof(defaultStepLength.c_str());
      optimizerValues[4] = defStep;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[5] = numIt;
      std::string useTrace = ReadXMLStringAttribut( "USETRACE", atts );
      double useTr = atof(useTrace.c_str());
      optimizerValues[6] = useTr;
    }
    else if (optimizer == mitk::OptimizerParameters::SPSAOPTIMIZER)
    {
      std::string a = ReadXMLStringAttribut( "a", atts );
      double a1 = atof(a.c_str());
      optimizerValues[2] = a1;
      std::string a2 = ReadXMLStringAttribut( "A", atts );
      double a3 = atof(a2.c_str());
      optimizerValues[3] = a3;
      std::string alpha = ReadXMLStringAttribut( "ALPHA", atts );
      double alp = atof(alpha.c_str());
      optimizerValues[4] = alp;
      std::string c = ReadXMLStringAttribut( "c", atts );
      double c1 = atof(c.c_str());
      optimizerValues[5] = c1;
      std::string gamma = ReadXMLStringAttribut( "GAMMA", atts );
      double gam = atof(gamma.c_str());
      optimizerValues[6] = gam;
      std::string tolerance = ReadXMLStringAttribut( "TOLERANCE", atts );
      double tol = atof(tolerance.c_str());
      optimizerValues[7] = tol;
      std::string stateOfConvergenceDecayRate = ReadXMLStringAttribut( "STATEOFCONVERGENCEDECAYRATE", atts );
      double stateOfConvergence = atof(stateOfConvergenceDecayRate.c_str());
      optimizerValues[8] = stateOfConvergence;
      std::string minNumberIterations = ReadXMLStringAttribut( "MINNUMBERITERATIONS", atts );
      double minNumIt = atof(minNumberIterations.c_str());
      optimizerValues[9] = minNumIt;
      std::string numberPerturbations = ReadXMLStringAttribut( "NUMBERPERTURBATIONS", atts );
      double numPer = atof(numberPerturbations.c_str());
      optimizerValues[10] = numPer;
      std::string numberIterations = ReadXMLStringAttribut( "NUMBERITERATIONS", atts );
      double numIt = atof(numberIterations.c_str());
      optimizerValues[11] = numIt;
    }
    return optimizerValues;
  }

  itk::Array<double> RigidRegistrationPreset::loadInterpolatorValues(itk::Array<double> interpolatorValues, double interpolator, const char **atts)
  {
    return interpolatorValues;
  }
}
