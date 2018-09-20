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

#ifndef __MITK_MODEL_FIT_PLOT_DATA_HELPER_H_
#define __MITK_MODEL_FIT_PLOT_DATA_HELPER_H_

#include <vector>

#include "mitkPoint.h"
#include "mitkPointSet.h"
#include "mitkModelBase.h"
#include "mitkWeakPointer.h"
#include "mitkCommon.h"

#include "mitkModelFitInfo.h"

#include "itkMapContainer.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class ModelParameterizerBase;
  class Image;

  using PlotDataValues = std::vector<std::pair<double, double>>;

  /** Simple helper structure that represents a curve for plots and its generation time*/
  class MITKMODELFIT_EXPORT PlotDataCurve : public ::itk::Object
  {
  public:
    mitkClassMacroItkParent(PlotDataCurve, itk::Object);
    itkFactorylessNewMacro(Self);

    using ValuesType = PlotDataValues;

    virtual void SetValues(const ValuesType& _arg);
    virtual void SetValues(ValuesType&& _arg);

    itkGetConstReferenceMacro(Values, ValuesType);
    itkGetMacro(Values, ValuesType);

    PlotDataCurve& operator=(const PlotDataCurve& rhs);
    PlotDataCurve& operator=(PlotDataCurve&& rhs) noexcept;

    void Reset();

  protected:
    PlotDataCurve();
    virtual ~PlotDataCurve() = default;

  private:
    /** values of the curve */
    ValuesType m_Values;

    PlotDataCurve(const PlotDataCurve& other) = delete;
  };

  /** Collection of plot curves, e.g. every plot curve for a certain world coordinate position*/
  using PlotDataCurveCollection = itk::MapContainer<std::string, PlotDataCurve::Pointer>;

  /** Structure containing all information for one model fit */
  struct MITKMODELFIT_EXPORT ModelFitPlotData
  {
    /** Plots that are related to the world coordinate labeled as current position.*/
    PlotDataCurveCollection::Pointer currentPositionPlots;

    using PositionalCurveCollection = std::pair<mitk::Point3D, PlotDataCurveCollection::Pointer>;
    using PositionalCollectionMap = std::map<mitk::PointSet::PointIdentifier, PositionalCurveCollection>;

    /** Plot collections that are related to specific world coordinates (inspection position bookmarks).*/
    PositionalCollectionMap positionalPlots;

    /** Plot collection for static plots of the fit that do not depend on some coordinates. */
    PlotDataCurveCollection::Pointer staticPlots;

    /** Pointer to the model fit that correspondens with this plot data.*/
    mitk::modelFit::ModelFitInfo::Pointer fitInfo;

    /** Helper function to get the collection of the current position.
    Returns nullptr if no current position exists.*/
    static const PlotDataCurve* GetSamplePlot(const PlotDataCurveCollection* coll);
    static const PlotDataCurve* GetSignalPlot(const PlotDataCurveCollection* coll);
    static const PlotDataCurve* GetInterpolatedSignalPlot(const PlotDataCurveCollection* coll);

    /** Helper function that generates a humand readable name for the passed value of a positional collection map.*/
    static std::string GetPositionalCollectionName(const PositionalCollectionMap::value_type& mapValue);

    const PlotDataCurveCollection* GetPositionalPlot(const mitk::Point3D& point) const;
    const PlotDataCurveCollection* GetPositionalPlot(mitk::PointSet::PointIdentifier id) const;

    /**returns the minimum (first element) and maximum (second element) of x of all plot data*/
    PlotDataValues::value_type GetXMinMax() const;
    /**returns the minimum (first element) and maximum (second element) of y of all plot data*/
    PlotDataValues::value_type GetYMinMax() const;

    ModelFitPlotData();
  };

  /** Helper function that actualizes min and max by the y values given in data.*/
  void CheckYMinMaxFromPlotDataValues(const PlotDataValues& data, double& min, double& max);

  /** Helper function that actualizes min and max by the x values given in data.*/
  void CheckXMinMaxFromPlotDataValues(const PlotDataValues& data, double& min, double& max);

  /** Function generates curve data for the signal defined by the passed informations.
   @param position The position in world coordinates the curve should be generated for.
   @param fitInfo Pointer to the fit info that defines the model/fit that produces the signal.
   @param timeGrid Defines the time grid of the generated signal.
   @param parameterizer Pointer to a parameterizer instance that is used to configure the model to generate the signal.
   If pointer is not set. The default parameterizer based on the fitInfo instance will be used.
   @pre position must be within the model fit input image
   @pre fitInfo must be a valid pointer.
   */
  MITKMODELFIT_EXPORT PlotDataCurve::Pointer
    GenerateModelSignalPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid, mitk::ModelParameterizerBase* parameterizer = nullptr);

  /** Function generates curve data for all additinal inputs (e.g. ROI signal, AIF)
  stored in the fit information. The keys in the map are the same keys like in the fitInfo.
  @param position The position in world coordinates the curve should be generated for.
  @param fitInfo Pointer to the fit info that defines the model/fit that produces the signal.
  @param timeGrid Defines the time grid of the generated signal.
  @pre position must be within the model fit input image
  @pre fitInfo must be a valid pointer.
  */
  MITKMODELFIT_EXPORT PlotDataCurveCollection::Pointer
    GenerateAdditionalModelFitPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid);

  /** Function generates curve data for a given image instance.
  @param position The position in world coordinates the curve should be generated for.
  @param image Pointer to the image the signal should be extracted from.
  @param timeGrid Defines the time grid of the generated signal.
  @pre position must be within the model fit input image
  @pre image must be a valid pointer.
  @pre image time steps must equal the timeGrid size.
  */
  MITKMODELFIT_EXPORT PlotDataCurve::Pointer
    GenerateImageSamplePlotData(const mitk::Point3D& position, const mitk::Image* image, const mitk::ModelBase::TimeGridType& timeGrid);

  /**
  * Keyword used in curve collections as key for the sample (extracted from an image) plot.
  */
  MITKMODELFIT_EXPORT const std::string MODEL_FIT_PLOT_SAMPLE_NAME();

  /**
  * Keyword used in curve collections as key for the signal (generated by a model) plot.
  */
  MITKMODELFIT_EXPORT const std::string MODEL_FIT_PLOT_SIGNAL_NAME();

  /**
  * Keyword used in curve collections as key for the interpolated (hires) signal (generated by a model) plot.
  */
  MITKMODELFIT_EXPORT const std::string MODEL_FIT_PLOT_INTERPOLATED_SIGNAL_NAME();

}

#endif
