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

#ifndef IGTLMeasurements_H_HEADER_INCLUDED_
#define IGTLMeasurements_H_HEADER_INCLUDED_

// Microservices
#include <mitkServiceInterface.h>
#include <usServiceRegistration.h>

#include "MitkOpenIGTLinkExports.h"
#include "itkObject.h"
#include "mitkCommon.h"

namespace mitk {

   /**
   * \brief Is a helper class to make measurments for latency and fps
   *
   * \ingroup OpenIGTLink
   */
   class MITKOPENIGTLINK_EXPORT IGTLMeasurementsImplementation : public itk::Object
   {
   public:
      mitkClassMacroItkParent(IGTLMeasurementsImplementation, itk::Object);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

         typedef std::list<long long>          MeasurementList;
      typedef std::map<unsigned int, MeasurementList> MeasurementPoints;

      /**
      * \brief AddMeasurementPoint
      */
      void AddMeasurement(unsigned int measurementPoint);
      bool ExportData(std::string filename);

      void RegisterAsMicroservice();
      void UnRegisterMicroservice();
   protected:
      IGTLMeasurementsImplementation();
      ~IGTLMeasurementsImplementation();

      /** \brief  */
      MeasurementPoints                                      m_MeasurementPoints;

      us::ServiceRegistration<Self> m_ServiceRegistration;
   };

   class MITKOPENIGTLINK_EXPORT IGTLMeasurements : public itk::Object
   {
   public:
      mitkClassMacroItkParent(IGTLMeasurements, itk::Object);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /**
      * \brief AddMeasurementPoint
      */
      void AddMeasurement(unsigned int measurementPoint);

      /**
      * \brief AddMeasurementPoint
      */
      bool ExportData(std::string filename);

      IGTLMeasurements();
      ~IGTLMeasurements();

   protected:
      IGTLMeasurementsImplementation::Pointer m_Measurements;
   };
} // namespace mitk
#endif /* MITKIGTLMeasurements_H_HEADER_INCLUDED_ */
