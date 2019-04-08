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

#ifndef MITKDIFFUSIONPROPERTYHELPER_H
#define MITKDIFFUSIONPROPERTYHELPER_H

#include <MitkDiffusionCoreExports.h>

#include <mitkImage.h>
#include <mitkBValueMapProperty.h>
#include <mitkGradientDirectionsProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <mitkDataNode.h>
#include <usModuleActivator.h>

namespace mitk
{
  /** \brief Helper class for mitk::Images containing diffusion weighted data
  *
  * This class takes a pointer to a mitk::Image containing diffusion weighted information and provides
  * functions to manipulate the diffusion meta-data. Will log an error if required information is
  * missing.
  */

  class MITKDIFFUSIONCORE_EXPORT DiffusionPropertyHelper
  {
  public:

    typedef short                                                         DiffusionPixelType;
    typedef mitk::BValueMapProperty::BValueMap                            BValueMapType;
    typedef GradientDirectionsProperty::GradientDirectionType             GradientDirectionType;
    typedef GradientDirectionsProperty::GradientDirectionsContainerType   GradientDirectionsContainerType;
    typedef mitk::MeasurementFrameProperty::MeasurementFrameType          MeasurementFrameType;
    typedef itk::VectorImage< DiffusionPixelType, 3>                      ImageType;

    static bool IsDiffusionWeightedImage(const mitk::Image *);
    static bool IsDiffusionWeightedImage(const mitk::DataNode* node);

    static void CopyProperties(mitk::Image* source, mitk::Image* target, bool ignore_original_gradients=false);

    static ImageType::Pointer GetItkVectorImage(Image *image);
    static const BValueMapType & GetBValueMap(const mitk::Image *);
    static float GetReferenceBValue(const mitk::Image *);
    static std::vector< int > GetBValueVector(const mitk::Image *);
    static const MeasurementFrameType & GetMeasurementFrame(const mitk::Image *);
    static GradientDirectionsContainerType::Pointer GetOriginalGradientContainer(const mitk::Image *);
    static GradientDirectionsContainerType::Pointer GetGradientContainer(const mitk::Image *);

    static void SetApplyMatrixToGradients(mitk::Image* image, bool apply);
    static void SetApplyMfToGradients(mitk::Image* image, bool apply);
    static void SetMeasurementFrame(mitk::Image* image, MeasurementFrameType mf);
    static void SetReferenceBValue(mitk::Image* image, float b_value);
    static void SetBValueMap(mitk::Image* image, BValueMapType map);
    static void SetOriginalGradientContainer(mitk::Image* image, GradientDirectionsContainerType::Pointer g_cont);
    static void SetGradientContainer(mitk::Image* image, GradientDirectionsContainerType::Pointer g_cont);
    static void RotateGradients(mitk::Image* image, vnl_matrix_fixed<double, 3, 3> rotation_matrix, bool normalize_columns);
    static void RotateOriginalGradients(mitk::Image* image, vnl_matrix_fixed<double, 3, 3> rotation_matrix, bool normalize_columns);

    static void AverageRedundantGradients(mitk::Image* image, double precision);
    static void InitializeImage(mitk::Image* image);

    static GradientDirectionsContainerType::Pointer CalcAveragedDirectionSet(double precision, GradientDirectionsContainerType::Pointer directions);

    static void SetupProperties(); // called in DiffusionCoreIOActivator

    static const std::string GetBvaluePropertyName() { return BVALUEMAPPROPERTYNAME; }
    static const std::string GetGradientContainerPropertyName() { return GRADIENTCONTAINERPROPERTYNAME; }
    static const std::string GetMeasurementFramePropertyName() { return MEASUREMENTFRAMEPROPERTYNAME; }

  protected:

    DiffusionPropertyHelper();
    ~DiffusionPropertyHelper();

    static const std::string GRADIENTCONTAINERPROPERTYNAME;
    static const std::string ORIGINALGRADIENTCONTAINERPROPERTYNAME;
    static const std::string MEASUREMENTFRAMEPROPERTYNAME;
    static const std::string REFERENCEBVALUEPROPERTYNAME;
    static const std::string BVALUEMAPPROPERTYNAME;
    static const std::string MODALITY;
    static const std::string APPLY_MATRIX_TO_GRADIENTS;
    static const std::string APPLY_MF_TO_GRADIENTS;

    /**
    * \brief Apply the previously set MeasurementFrame and the image rotation matrix to all gradients
    *
    * \warning first set the MeasurementFrame
    */
    static void ApplyMeasurementFrameAndRotationMatrix(mitk::Image* image);

    /**
    * \brief Apply the inverse of the previously set MeasurementFrame and the image rotation matrix to all gradients
    *
    * \warning first set the MeasurementFrame
    */
    static void UnApplyMeasurementFrameAndRotationMatrix(mitk::Image* image);

    /**
    * \brief Update the BValueMap (m_B_ValueMap) using the current gradient directions (m_Directions)
    *
    * \warning Have to be called after each manipulation on the GradientDirectionContainer
    * !especially after manipulation of the m_Directions (GetDirections()) container via pointer access!
    */
    static void UpdateBValueMap(mitk::Image* image);

    /// Determines whether gradients can be considered to be equal
    static bool AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision);

    /// Get the b value belonging to an index
    static float GetB_Value(const mitk::Image* image, unsigned int i);
  };
}
#endif
