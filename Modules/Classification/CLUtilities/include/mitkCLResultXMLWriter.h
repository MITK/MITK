/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCLResultXMLWriter_h
#define mitkCLResultXMLWriter_h

#include "MitkCLUtilitiesExports.h"

#include "mitkImage.h"
#include "mitkAbstractGlobalImageFeature.h"

namespace mitk
{
  namespace cl
  {
    class MITKCLUTILITIES_EXPORT CLResultXMLWriter
    {
    public:
      CLResultXMLWriter() = default;
      ~CLResultXMLWriter() = default;

      void SetImage(const Image* image);
      void SetMask(const Image* mask);
      void SetFeatures(const mitk::AbstractGlobalImageFeature::FeatureListType& features);

      void SetMethodName(const std::string& name);
      void SetMethodVersion(const std::string& version);
      void SetOrganisation(const std::string& orga);
      void SetPipelineUID(const std::string& pipelineUID);
      void SetCLIArgs(const std::map<std::string, us::Any>& args);

      void write(const std::string& filePath) const;
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
