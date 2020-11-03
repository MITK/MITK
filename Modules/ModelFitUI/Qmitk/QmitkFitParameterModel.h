/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFitParameterModel_h
#define QmitkFitParameterModel_h

#include <QAbstractTableModel>

#include "mitkModelFitInfo.h"
#include "mitkPointSet.h"

#include "MitkModelFitUIExports.h"


/*!
\class QmitkFitParameterModel
Model that can be used to display the parameter values of ModelFitInfo instances for different world coordinate positions.
If more then one ModelFitInfo instance is given the model will use a tree hirarchy. The first level are the fits,
the seconds level are the parameter of the fit.
*/
class MITKMODELFITUI_EXPORT QmitkFitParameterModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  QmitkFitParameterModel(QObject* parent = nullptr);
  ~QmitkFitParameterModel() override {};

  const FitVectorType& getFits() const;

  mitk::Point3D getCurrentPosition() const;

  const mitk::PointSet* getPositionBookmarks() const;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

public Q_SLOTS:

  void setFits(const FitVectorType& fits);

  void setCurrentPosition(const mitk::Point3D& currentPos);

  void setPositionBookmarks(const mitk::PointSet* bookmarks);


protected:

  std::size_t getBookmarksCount() const;

private:

  bool hasSingleFit() const;

  FitVectorType m_Fits;
  mitk::PointSet::ConstPointer m_Bookmarks;
  mitk::Point3D m_CurrentPos;

};

#endif // QmitkFitParameterModel_h

