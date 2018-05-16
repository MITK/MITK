
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

#ifndef _MITK_CORRELATIONCALCULATOR_H
#define _MITK_CORRELATIONCALCULATOR_H

#include "mitkCommon.h"
#include <itkObject.h>
#include <mitkImage.h>
#include <vnl/vnl_matrix.h>
#include <mitkConnectomicsNetwork.h>

namespace mitk {

  /**
    * \brief Class to calculate covariance and correlation
    *
    * In addition to static functions to calculate covariance and correlation,
    * this class offers capability to set a time series image and optionally a
    * parcellation image and calculate a correlation matrix.
    *
    * Whole correlation only needs a time series image and will return a
    * #voxel x #voxel matrix where each voxel's time series is correlated
    * against the time series of each other voxel.
    *
    * Parcel correlation will compute an average time series for each parcel
    * of a parcellation image and create a #parcel x #parcel correlation matrix.
    *
    * Please note that time series images are assumed to be 4D and parcellation
    * images integer valued and 3D.
    */

  template< class T >
  class CorrelationCalculator : public itk::Object
  {

  public:

    // Standard macros
    mitkClassMacroItkParent(CorrelationCalculator, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * @brief The ParcelMode enum defines how to define the correlation between two parcels
     *
     * Available choices are:
     * <ol>
     *  <li> UseAverageTimeSeries - Average time series across the parcel, correlate average time series between parcels
     *  <li> UseMaximumCorrelation - Correlate all voxels of the parcels with each other, use maximum correlation
     *  <li> UseAverageCorrelation - Correlate all voxels of the parcels with each other, use average correlation
     * </ol>
     */
    enum ParcelMode
    {
      UseAverageTimeSeries,
      UseMaximumCorrelation,
      UseAverageCorrelation
    };

    /**
     * @brief Calculate and return the pearson correlation coefficient for two vectors of data
     *
     * This function expects two vectors of the same length and data type and calculates the
     * pearson correlation coefficient.
     *
     * @param one first data vector
     * @param two second data vector
     * @return pearson correlation coefficent
     */
    static double CalculatePearsonCorrelationCoefficient( const std::vector<T>& one, const std::vector<T>& two );

    /**
     * @brief Calculates the covariance of two vectors of data
     *
     * Expects two vectors of the same length and data type and returns the covariance of the two.
     * Variance can be calculated by providing the same data vector twice.
     *
     * @param one first data vector
     * @param two second data vector
     * @return covariance of the two
     */
    static double CalculateCovariance( const std::vector<T>& one, const std::vector<T>& two );

    /**
     * @brief Extracts the average time series from a roi defined by the parcellation image
     *
     * This function requires a parcellation image and a time series image to be set and registered to each other.
     * For each voxel in the time series image it will check whether it is inside the specified parcel of the
     * parcellation image. All inside voxels will be averaged for all time steps and an average time series
     * is returned.
     *
     * Calculates and saves the averages for all parcellations, to speed up consecutive calls.
     *
     * @param parcelValue the voxel value of the roi in the parcellation image.
     * @return vector of the average values of the roi for each timestep
     */
    std::vector< double > ExtractAverageTimeSeriesOfParcel( int parcelValue );

    /**
     * @brief Create a #voxel x #voxel correlation matrix
     */
    void DoWholeCorrelation();

    /**
     * @brief Create a #parcel x #parcel correlation matrix
     */
    void DoParcelCorrelation( ParcelMode mode = UseAverageTimeSeries );

    /**
     * @brief Marks average time series as invalid and calls superclass modified
     */
    virtual void Modified();

    const vnl_matrix< double >* GetCorrelationMatrix() const;
    const std::map< unsigned int, int >* GetLabelOrderMap() const;
    mitk::ConnectomicsNetwork::Pointer GetConnectomicsNetwork();

    // Get/Set macros
    itkSetMacro(ParcellationImage, mitk::Image::Pointer)
    itkSetMacro(TimeSeriesImage, mitk::Image::Pointer)

  protected:

    // intentionally not accessible, use our non-const Modified function instead
    void Modified() const override;

    CorrelationCalculator();
    ~CorrelationCalculator() override;

    template< typename TPixel, unsigned int VImageDimension >
    void ExtractAllAverageTimeSeries( itk::Image< TPixel, VImageDimension>* itkTimeSeriesImage );

    void ExtractCenterOfMassForParcels();


    double GetParcelCorrelation(const int& parcelA,const int& parcelB, ParcelMode& mode) const;

    // Member variables
    bool m_InvalidTimeSeries;

    mitk::Image::Pointer m_ParcellationImage;
    mitk::Image::Pointer m_TimeSeriesImage;

    std::map< int, std::vector<double> > m_AverageTimeSeries;
    std::map< int, std::vector<std::vector< T > > > m_ParcelTimeSeries;

    std::map< int, mitk::Point3D > m_ParcelCenterOfMass;

    vnl_matrix< double > m_CorrelationMatrix;

    std::map< unsigned int, int > m_LabelOrderMap;
  };

}

#include <mitkCorrelationCalculator.txx>

#endif /* _MITK_CORRELATIONCALCULATOR_H */
