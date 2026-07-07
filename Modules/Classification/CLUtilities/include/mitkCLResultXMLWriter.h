/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCLResultXMLWriter_h
#define mitkCLResultXMLWriter_h

#include <MitkCLUtilitiesExports.h>

#include <mitkImage.h>
#include <mitkAbstractGlobalImageFeature.h>

namespace mitk
{
  namespace cl
  {
    /**
     * \brief Writes classification feature results to an XML file.
     *
     * Serializes image metadata, mask information, computed features, and pipeline
     * metadata (method name, version, organisation) into a structured XML format.
     */
    class MITKCLUTILITIES_EXPORT CLResultXMLWriter
    {
    public:
      CLResultXMLWriter() = default;
      ~CLResultXMLWriter() = default;

      /**
       * \brief Set the intensity image whose features were computed.
       * \param image Pointer to the input image.
       */
      void SetImage(const Image* image);

      /**
       * \brief Set the mask image used for feature computation.
       * \param mask Pointer to the mask image.
       */
      void SetMask(const Image* mask);

      /**
       * \brief Set the computed feature list to be written.
       * \param features The list of feature name-value pairs.
       */
      void SetFeatures(const mitk::AbstractGlobalImageFeature::FeatureListType& features);

      /**
       * \brief Set the name of the analysis method.
       * \param name The method name string.
       */
      void SetMethodName(const std::string& name);

      /**
       * \brief Set the version of the analysis method.
       * \param version The version string.
       */
      void SetMethodVersion(const std::string& version);

      /**
       * \brief Set the organisation performing the analysis.
       * \param orga The organisation name.
       */
      void SetOrganisation(const std::string& orga);

      /**
       * \brief Set the unique identifier of the analysis pipeline.
       * \param pipelineUID The pipeline UID string.
       */
      void SetPipelineUID(const std::string& pipelineUID);

      /**
       * \brief Set the command-line arguments used for the analysis.
       * \param args Map of argument names to values.
       */
      void SetCLIArgs(const std::map<std::string, us::Any>& args);

      /**
       * \brief Write the XML output to a file.
       * \param filePath Path to the output XML file.
       */
      void write(const std::string& filePath) const;

      /**
       * \brief Write the XML output to a stream.
       * \param stream The output stream to write to.
       */
      void write(std::ostream& stream) const;

    private:
      CLResultXMLWriter(const CLResultXMLWriter&) = delete;
      CLResultXMLWriter& operator = (const CLResultXMLWriter&) = delete;

      Image::ConstPointer m_Image;
      Image::ConstPointer m_Mask;
      mitk::AbstractGlobalImageFeature::FeatureListType m_Features;
      std::string m_MethodName = "unknown";
      std::string m_MethodVersion = "unknown";
      std::string m_Organisation = "unknown";
      std::string m_PipelineUID = "unknown";
      std::map<std::string, us::Any> m_CLIArgs;
    };
  }
}

#endif
