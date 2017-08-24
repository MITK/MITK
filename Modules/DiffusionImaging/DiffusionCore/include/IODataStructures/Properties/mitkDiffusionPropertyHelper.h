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

    static const std::string GRADIENTCONTAINERPROPERTYNAME;
    static const std::string ORIGINALGRADIENTCONTAINERPROPERTYNAME;
    static const std::string MEASUREMENTFRAMEPROPERTYNAME;
    static const std::string REFERENCEBVALUEPROPERTYNAME;
    static const std::string BVALUEMAPPROPERTYNAME;
    static const std::string MODALITY;
    static const std::string KEEP_ORIGINAL_DIRECTIONS;

    /// Public constructor, takes a mitk::Image pointer as argument
    DiffusionPropertyHelper( mitk::Image* inputImage );
    ~DiffusionPropertyHelper();

    /** \brief Decide whether a provided image is a valid diffusion weighted image
     *
     * An image will be considered a valid diffusion weighted image if the following are true
     * - It has a reference b value
     * - It has a gradient directions property
     * - The number of gradients directions matches the number of components of the image
     *
     * This does not guarantee that the data is sensible or accurate, it just verfies that it
     * meets the formal requirements to possibly be a valid diffusion weighted image.
     */
    static bool IsDiffusionWeightedImage(const mitk::Image *);
    static bool IsDiffusionWeightedImage(const mitk::DataNode* node);

    static void ClearMeasurementFrameAndRotationMatrixFromGradients(mitk::Image* image);
    static void CopyProperties(mitk::Image* source, mitk::Image* target, bool ignore_original_gradients=false);

    static ImageType::Pointer GetItkVectorImage(Image *image);

    /// Convenience method to get the BValueMap
    static const BValueMapType & GetBValueMap(const mitk::Image *);
    /// Convenience method to get the BValue
    static float GetReferenceBValue(const mitk::Image *);
    /** \brief Convenience method to get the measurement frame
     *
     * This method will return the measurement frame of the image.
     *
     * \note If the image has no associated measurement frame the identity will be returned.
     */
    static const MeasurementFrameType & GetMeasurementFrame(const mitk::Image *);
    /// Convenience method to get the original gradient directions
    static GradientDirectionsContainerType::Pointer GetOriginalGradientContainer(const mitk::Image *);
    /// Convenience method to get the gradient directions
    static GradientDirectionsContainerType::Pointer GetGradientContainer(const mitk::Image *);

    const BValueMapType & GetBValueMap() const;
    float GetReferenceBValue() const;
    const MeasurementFrameType & GetMeasurementFrame() const;
    GradientDirectionsContainerType::Pointer GetOriginalGradientContainer() const;
    GradientDirectionsContainerType::Pointer GetGradientContainer() const;

    bool IsDiffusionWeightedImage() const;

    void AverageRedundantGradients(double precision);

    /** \brief Make certain the owned image is up to date with all necessary properties
     *
     * This function will generate the B Value map and copy all properties to the owned image.
     */
    void InitializeImage();

    GradientDirectionsContainerType::Pointer CalcAveragedDirectionSet(double precision, GradientDirectionsContainerType::Pointer directions);

  protected:

    DiffusionPropertyHelper();

    /**
    * \brief Apply the previously set MeasurementFrame and the image rotation matrix to all gradients
    *
    * \warning first set the MeasurementFrame
    */
    void ApplyMeasurementFrameAndRotationMatrix();

    /**
    * \brief Apply the inverse of the previously set MeasurementFrame and the image rotation matrix to all gradients
    *
    * \warning first set the MeasurementFrame
    */
    void UnApplyMeasurementFrameAndRotationMatrix();

    /**
    * \brief Update the BValueMap (m_B_ValueMap) using the current gradient directions (m_Directions)
    *
    * \warning Have to be called after each manipulation on the GradientDirectionContainer
    * !especially after manipulation of the m_Directions (GetDirections()) container via pointer access!
    */
    void UpdateBValueMap();

    /// Determines whether gradients can be considered to be equal
    bool AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision);

    /// Get the b value belonging to an index
    float GetB_Value(unsigned int i);

    mitk::Image* m_Image;

  };
}
#endif
