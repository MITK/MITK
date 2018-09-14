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

  QmitkFitParameterModel(QObject* parent = NULL);
  virtual ~QmitkFitParameterModel() {};

  const FitVectorType& getFits() const;

  mitk::Point3D getCurrentPosition() const;

  const mitk::PointSet* getPositionBookmarks() const;

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

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

