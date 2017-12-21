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

#ifndef MITKPHOTOACOUSTICPROBE_H
#define MITKPHOTOACOUSTICPROBE_H

#include <MitkPhotoacousticsLibExports.h>

#include <mitkCommon.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

#include "mitkPALightSource.h"
#include <vector>
#include <tinyxml.h>

namespace mitk {
  namespace pa {
    /**
     * @brief The Probe class
     * The representation of a Probe
     */
    class MITKPHOTOACOUSTICSLIB_EXPORT Probe : public itk::LightObject
    {
    public:

      mitkClassMacroItkParent(Probe, itk::LightObject)
        mitkNewMacro2Param(Self, std::string, bool)
        mitkNewMacro2Param(Self, const char*, bool)

        const std::string XML_TAG_PROBE = "Probe";
      const std::string XML_TAG_LIGHT_SOURCE = "LightSource";

      LightSource::PhotonInformation GetNextPhoton(double rng1, double rnd2, double rnd3, double rnd4,
        double rnd5, double rnd6, double rnd7, double rnd8);

      bool IsValid();

      Probe(std::string xmlFile, bool verbose);
      Probe(const char* fileStream, bool verbose);
      ~Probe() override;

    protected:

      std::vector<LightSource::Pointer> m_LightSources;
      bool m_IsValid;
      double m_TotalEnergy;
      bool m_Verbose;

      void InitProbe(TiXmlDocument document);
    };
  }
}
#endif // MITKPHOTOACOUSTICPROBE_H
