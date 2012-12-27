
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

#ifndef _MITK_ConnectomicsHistogramBase_H
#define _MITK_ConnectomicsHistogramBase_H

#include "mitkSimpleHistogram.h"
#include "mitkCommon.h"

#include "mitkConnectomicsNetwork.h"

namespace mitk {

  /**
    * \brief Superclass for histograms working with connectomic networks
    */

  class ConnectomicsHistogramBase : public mitk::SimpleHistogram
  {

  public:

    ConnectomicsHistogramBase();
    virtual ~ConnectomicsHistogramBase();

    /** @brief Returns the minimal y=f(x) value of the histogram. */
    virtual double GetYMin() const;
    /** @brief Returns the maximum y=f(x) value of the histogram. */
    virtual double GetYMax() const;
    /** @brief Returns the minimal x value of the histogram. */
    virtual double GetXMin() const;
    /** @brief Returns the maximum x value of the histogram. */
    virtual double GetXMax() const;
    /** @brief Returns the range of the histogram. */
    virtual int GetRange() const;
    /** @brief Update the Y maximum to the maximal value in the histogram */
    virtual void UpdateYMax();
    /** @brief Creates a new histogram from the source. */
    virtual void ComputeFromBaseData( BaseData* source );
    /** @brief Print values to console. */
    virtual void PrintToConsole( ) const;
    /** @brief Returns whether the histogram can be considered valid. */
    virtual bool IsValid() const;
    /** @brief Returns the subject of the histogram as a string. */
    virtual std::string GetSubject() const;
    /** @brief Set the subject of the histogram as a string. */
    virtual void SetSubject( std::string );

    /** @brief Get bin height for the bin between start and end*/
    virtual float GetRelativeBin( double start, double end ) const;

    /** @brief Get the double vector*/
    virtual std::vector< double > GetHistogramVector();

  protected:

    // Functions

    /** @brief Creates a new histogram from the network source. */
    virtual void ComputeFromConnectomicsNetwork( ConnectomicsNetwork* source ) = 0;
    /** @brief Legacy method, do no use */
    virtual double GetMin() const;
    /** @brief Legacy method, do no use */
    virtual double GetMax() const;

    // Variables

    /** @brief Is this a valid histogram*/
    bool m_Valid;
    /** @brief Which is the baseline value for the histogram
    /*
    /* This value should be zero for all network histograms */
    int m_BaselineValue;
    /** @brief Which is the top value for the histogram */
    int m_TopValue;
    /** @brief Which is the starting x value for the histogram */
    int m_StartValue;

    /** @brief We expect not continuous but discrete histograms */
    std::vector< double > m_HistogramVector;

    /** @brief Subject of the histogram as a string. */
    std::string m_Subject;
  };

}

#endif /* _MITK_ConnectomicsHistogramBase_H */
