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

#ifndef mitkConnectomicsStatisticsCalculator_h
#define mitkConnectomicsStatisticsCalculator_h

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMacro.h>

#include "mitkCommon.h"

#include "ConnectomicsExports.h"

#include <mitkConnectomicsNetwork.h>

namespace mitk
{
  /**
  * \brief A class giving functions for calculating a variety of network indices */
  class Connectomics_EXPORT ConnectomicsStatisticsCalculator : public itk::Object
  {
  public:

    /** Standard class typedefs. */
    /** Method for creation through the object factory. */

    mitkClassMacro(ConnectomicsStatisticsCalculator, itk::Object);
    itkNewMacro(Self);

    itkSetObjectMacro( Network, mitk::ConnectomicsNetwork );

    itkGetMacro( NumberOfVertices, unsigned int );
    itkGetMacro( NumberOfEdges, unsigned int );
    itkGetMacro( AverageDegree, double );
    itkGetMacro( ConnectionDensity, double );
    itkGetMacro( NumberOfConnectedComponents, unsigned int );
    itkGetMacro( AverageComponentSize, double );

    void Update();

  protected:

    //////////////////// Functions ///////////////////////
    ConnectomicsStatisticsCalculator();
    ~ConnectomicsStatisticsCalculator();

    void CalculateNumberOfVertices();

    void CalculateNumberOfEdges();

    void CalculateAverageDegree();

    void CalculateConnectionDensity();

    void CalculateNumberOfConnectedComponents();

    void CalculateAverageComponentSize();

    /////////////////////// Variables ////////////////////////

    // The connectomics network, which is used for statistics calculation
    mitk::ConnectomicsNetwork::Pointer m_Network;

    // Statistics
    unsigned int m_NumberOfVertices;
    unsigned int m_NumberOfEdges;
    double m_AverageDegree;
    double m_ConnectionDensity;
    unsigned int m_NumberOfConnectedComponents;
    double m_AverageComponentSize;
  };

}// end namespace mitk

#endif // mitkConnectomicsStatisticsCalculator_h
