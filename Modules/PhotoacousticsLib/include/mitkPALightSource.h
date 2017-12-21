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

#ifndef MITKPHOTOACOUSTICLIGHTSOURCE_H
#define MITKPHOTOACOUSTICLIGHTSOURCE_H

#include <mitkCommon.h>

#include "MitkPhotoacousticsLibExports.h"

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkObject.h"
#include "itkMacro.h"

#include <tinyxml.h>

namespace mitk {
  namespace pa {
    /**
     * @brief The LightSource class
     * The representation of a LightSource
     */
    class MITKPHOTOACOUSTICSLIB_EXPORT LightSource : public itk::Object
    {
    public:

      mitkClassMacroItkParent(LightSource, itk::Object)
        itkFactorylessNewMacro(Self)
        mitkNewMacro2Param(Self, TiXmlElement*, bool)

        const std::string XML_TAG_X_ANGLE = "xAngle";
      const std::string XML_TAG_Y_ANGLE = "yAngle";
      const std::string XML_TAG_PHOTON_DIRECTION = "PhotonDirection";
      const std::string XML_TAG_MINIMUM = "min";
      const std::string XML_TAG_MAXIMUM = "max";
      const std::string XML_TAG_MODE = "mode";
      const std::string XML_TAG_ENERGY = "energy";
      const std::string XML_TAG_SPAWN_TYPE = "SpawnType";
      const std::string XML_TAG_SPAWN_TYPE_POINT = "POINT";
      const std::string XML_TAG_SPAWN_TYPE_RECTANGLE = "RECTANGLE";
      const std::string XML_TAG_SPAWN_TYPE_CIRCLE = "CIRCLE";
      const std::string XML_TAG_X = "x";
      const std::string XML_TAG_Y = "y";
      const std::string XML_TAG_Z = "z";
      const std::string XML_TAG_R = "r";
      const std::string XML_TAG_X_LENGTH = "xLength";
      const std::string XML_TAG_Y_LENGTH = "yLength";
      const std::string XML_TAG_Z_LENGTH = "zLength";

      enum SpawnType
      {
        POINT, RECTANGLE, CIRCLE
      };

      enum DistributionMode
      {
        UNIFORM, GAUSSIAN
      };

      struct PhotonInformation
      {
        double xPosition;
        double yPosition;
        double zPosition;
        double xAngle;
        double yAngle;
        double zAngle;
      };

      PhotonInformation GetNextPhoton(double rnd1, double rnd2, double rnd3,
        double rnd4, double rnd5, double gau1, double gau2);

      bool IsValid();

      LightSource(TiXmlElement* element, bool verbose);
      LightSource();
      ~LightSource() override;

      void ParseAngle(TiXmlElement* direction, std::string angle);

      itkGetMacro(SpawnType, SpawnType)
        itkSetMacro(SpawnType, SpawnType)

        itkGetMacro(SpawnLocationX, double)
        itkSetMacro(SpawnLocationX, double)

        itkGetMacro(SpawnLocationY, double)
        itkSetMacro(SpawnLocationY, double)

        itkGetMacro(SpawnLocationZ, double)
        itkSetMacro(SpawnLocationZ, double)

        itkGetMacro(SpawnLocationXLength, double)
        itkSetMacro(SpawnLocationXLength, double)

        itkGetMacro(SpawnLocationYLength, double)
        itkSetMacro(SpawnLocationYLength, double)

        itkGetMacro(SpawnLocationZLength, double)
        itkSetMacro(SpawnLocationZLength, double)

        itkGetMacro(SpawnLocationRadius, double)
        itkSetMacro(SpawnLocationRadius, double)

        itkGetMacro(Energy, double)
        itkSetMacro(Energy, double)

        itkGetMacro(AngleXMinimum, double)
        itkSetMacro(AngleXMinimum, double)

        itkGetMacro(AngleXMaximum, double)
        itkSetMacro(AngleXMaximum, double)

        itkGetMacro(AngleYMinimum, double)
        itkSetMacro(AngleYMinimum, double)

        itkGetMacro(AngleYMaximum, double)
        itkSetMacro(AngleYMaximum, double)

        itkGetMacro(AngleXMode, DistributionMode)
        itkSetMacro(AngleXMode, DistributionMode)

        itkGetMacro(AngleYMode, DistributionMode)
        itkSetMacro(AngleYMode, DistributionMode)

        itkGetMacro(Verbose, bool)
        itkSetMacro(Verbose, bool)

    protected:

      const double TWO_PI = 2.0*3.14159265358979323846;

      SpawnType m_SpawnType;
      double m_SpawnLocationX;
      double m_SpawnLocationY;
      double m_SpawnLocationZ;
      double m_SpawnLocationXLength;
      double m_SpawnLocationYLength;
      double m_SpawnLocationZLength;
      double m_SpawnLocationRadius;

      double m_Energy;

      double m_AngleXMinimum;
      double m_AngleXMaximum;
      double m_AngleYMinimum;
      double m_AngleYMaximum;
      DistributionMode m_AngleXMode;
      DistributionMode m_AngleYMode;

      bool m_IsValid;
      bool m_Verbose;

      struct TransformResult
      {
        double z0;
        double z1;
      };

      void ParsePhotonDirection(TiXmlElement* element);
      void ParseEnergy(TiXmlElement* element);
      void ParsePhotonSpawnArea(TiXmlElement* element);

      TransformResult BoxMuellerTransform(double u1, double u2, double mu, double sigma);
    };
  }
}
#endif // MITKPHOTOACOUSTICLIGHTSOURCE_H
