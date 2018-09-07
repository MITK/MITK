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
#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  class ModelParameterizerBase;
  class Image;

  namespace modelFit
  {
    class ModelFitInfo;

  }

  using PlotDataValues = std::vector<std::pair<double, double>>;

  /** Simple helper structure that represents a curve for plots and its generation time*/
  struct MITKMODELFIT_EXPORT PlotDataCurve
  {
    /** values of the curve */
    PlotDataValues values;

    /** Timestamp indicating the last change of the values.*/
    itk::TimeStamp time;

    PlotDataCurve() = default;
    PlotDataCurve(const PlotDataValues& values);
    PlotDataCurve(const PlotDataCurve& other);
    PlotDataCurve(PlotDataCurve&& other);

    PlotDataCurve& operator=(const PlotDataCurve& rhs);
    PlotDataCurve& operator=(PlotDataCurve&& rhs) noexcept;

    void Reset();
  };

  /** Collection of plot curves, e.g. every plot curve for a certain world coordinate position*/
  using PlotDataCurveCollection = std::map<std::string, PlotDataCurve>;

  using PlotDataCurveCollectionMap = std::map<std::string, PlotDataCurveCollection>;


  /** Function generates curve data for the signal defined by the passed informations.
   @param position The position in world coordinates the curve should be generated for.
   @param fitInfo Pointer to the fit info that defines the model/fit that produces the signal.
   @param timeGrid Defines the time grid of the generated signal.
   @param parameterizer Pointer to a parameterizer instance that is used to configure the model to generate the signal.
   If pointer is not set. The default parameterizer based on the fitInfo instance will be used.
   @pre position must be within the model fit input image
   @pre fitInfo must be a valid pointer.
   */
  MITKMODELFIT_EXPORT PlotDataCurve
    GenerateModelSignalPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid, mitk::ModelParameterizerBase* parameterizer = nullptr);

  /** Function generates curve data for all additinal inputs (e.g. ROI signal, AIF)
  stored in the fit information. The keys in the map are the same keys like in the fitInfo.
  @param position The position in world coordinates the curve should be generated for.
  @param fitInfo Pointer to the fit info that defines the model/fit that produces the signal.
  @param timeGrid Defines the time grid of the generated signal.
  @pre position must be within the model fit input image
  @pre fitInfo must be a valid pointer.
  */
  MITKMODELFIT_EXPORT PlotDataCurveCollection
    GenerateAdditionalModelFitPlotData(const mitk::Point3D& position, const mitk::modelFit::ModelFitInfo* fitInfo, const mitk::ModelBase::TimeGridType& timeGrid);

  /** Function generates curve data for a given image instance.
  @param position The position in world coordinates the curve should be generated for.
  @param image Pointer to the image the signal should be extracted from.
  @param timeGrid Defines the time grid of the generated signal.
  @pre position must be within the model fit input image
  @pre image must be a valid pointer.
  @pre image time steps must equal the timeGrid size.
  */
  MITKMODELFIT_EXPORT PlotDataCurve
    GenerateImageSamplePlotData(const mitk::Point3D& position, const mitk::Image* image, const mitk::ModelBase::TimeGridType& timeGrid);

}

#endif
