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

   ///**
   //* \brief Is a helper class to make measurments for latency and fps
   //*
   //* \ingroup OpenIGTLink
   //*/
   //class MITKOPENIGTLINK_EXPORT IGTLMeasurementsImplementation : public itk::Object
   //{
   //public:
   //   mitkClassMacroItkParent(IGTLMeasurementsImplementation, itk::Object);
   //   itkFactorylessNewMacro(Self)
   //   itkCloneMacro(Self)

   //   typedef std::list<long long>                    MeasurementList;
   //   typedef std::map<unsigned int, MeasurementList> MeasurementPoints;

   //   /**
   //   * \brief AddMeasurementPoint
   //   */
   //   void AddMeasurement(unsigned int measurementPoint);
   //   bool ExportData(std::string filename);

   //   /**
   //   * \brief clears all measurements
   //   */
   //   void Reset();

   //   void SetStarted(bool started);

   //   void RegisterAsMicroservice();
   //   void UnRegisterMicroservice();
   //protected:
   //   IGTLMeasurementsImplementation();
   //   ~IGTLMeasurementsImplementation();

   //   /** \brief  */
   //   MeasurementPoints                                      m_MeasurementPoints;

   //   bool m_IsStarted;

   //   us::ServiceRegistration<Self> m_ServiceRegistration;
   //};

  /**
  * \brief Is a helper class to make measurments for latency and fps
  *
  * \ingroup OpenIGTLink
  */
  class MITKOPENIGTLINK_EXPORT IGTLMeasurements : public itk::Object
  {
  public:
    mitkClassMacroItkParent(IGTLMeasurements, itk::Object);
    static IGTLMeasurements* GetInstance();

    /**
    * \brief AddMeasurementPoint
    * \param timestamp Sets the timestamp, if it is 0 the current system time is used.
    */
    void AddMeasurement(unsigned int measurementPoint, unsigned int index, long long timestamp = 0);


    /**
    * \brief AddMeasurementPoint
    */
    bool ExportData(std::string filename);

    /**
    * \brief clears all measurements
    */
    void Reset();

    void SetStarted(bool started);

  private:
    // Only our module activator class should be able to instantiate
    // a SingletonOneService object.
    friend class IGTLModuleActivator;

    itkFactorylessNewMacro(Self)
    //itkCloneMacro(Self)

    IGTLMeasurements();
    ~IGTLMeasurements();

    // Disable copy constructor and assignment operator.
    IGTLMeasurements(const IGTLMeasurements&);
    IGTLMeasurements& operator=(const IGTLMeasurements&);

    //protected:
    //IGTLMeasurementsImplementation::Pointer m_Measurements;

    typedef std::pair<long long, unsigned int>      TimeStampIndexPair;
    typedef std::list<TimeStampIndexPair>           MeasurementList;
    typedef std::map<unsigned int, MeasurementList> MeasurementPoints;

    MeasurementPoints                               m_MeasurementPoints;

    bool m_IsStarted;
  };
} // namespace mitk
#endif /* MITKIGTLMeasurements_H_HEADER_INCLUDED_ */
