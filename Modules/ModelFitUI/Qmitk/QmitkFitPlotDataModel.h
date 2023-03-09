/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFitPlotDataModel_h
#define QmitkFitPlotDataModel_h

#include <QAbstractTableModel>

#include "mitkModelFitPlotDataHelper.h"
#include "MitkModelFitUIExports.h"

/*!
\class QmitkFitPlotDataModel
Model that can be used to display the values of an ModelFitPlotData instance.
*/
class MITKMODELFITUI_EXPORT QmitkFitPlotDataModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  QmitkFitPlotDataModel(QObject* parent = nullptr);
  ~QmitkFitPlotDataModel() override {};

  const mitk::ModelFitPlotData* GetPlotData() const;
  const std::string& GetXName() const;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

public Q_SLOTS:

  void SetPlotData(const mitk::ModelFitPlotData* data);
  void SetXName(const std::string& xName);

protected:
  std::pair<std::string, const mitk::PlotDataCurve*> GetCurveByColumn(int col) const;
  std::pair<bool, mitk::ModelFitPlotData::PositionalCollectionMap::value_type> GetPositionalCurvePoint(const mitk::PlotDataCurve*) const;

private:
  mitk::ModelFitPlotData m_PlotData;
  std::string m_XName;

};

#endif
