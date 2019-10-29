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

#include "mitkExceptionMacro.h"
#include "mitkModelFitParameterValueExtraction.h"

#include "QmitkFitParameterModel.h"


QmitkFitParameterModel::
QmitkFitParameterModel(QObject* parent) :
  QAbstractTableModel(parent)
{
  m_CurrentPos.Fill(0.0);
}

const QmitkFitParameterModel::FitVectorType&
QmitkFitParameterModel::
getFits() const
{
  return m_Fits;
};

mitk::Point3D
QmitkFitParameterModel::
getCurrentPosition() const
{
  return m_CurrentPos;
};

const mitk::PointSet*
QmitkFitParameterModel::
getPositionBookmarks() const
{
  return m_Bookmarks;
};

void
QmitkFitParameterModel::
setFits(const FitVectorType& fits)
{
  emit beginResetModel();
  m_Fits = fits;
  emit endResetModel();
};

void
QmitkFitParameterModel::
setCurrentPosition(const mitk::Point3D& currentPos)
{
  emit beginResetModel();
  m_CurrentPos = currentPos;
  emit endResetModel();
};

void
QmitkFitParameterModel::
setPositionBookmarks(const mitk::PointSet* bookmarks)
{
  emit beginResetModel();
  m_Bookmarks = bookmarks;
  emit endResetModel();
};

bool
QmitkFitParameterModel::
hasSingleFit() const
{
  return this->m_Fits.size() == 1;
};

int
QmitkFitParameterModel::
rowCount(const QModelIndex& parent) const
{
  if (this->hasSingleFit())
  {
    if (parent.isValid())
    {
      return 0;
    }
    else
    {
      return this->m_Fits.front()->GetParameters().size() + this->m_Fits.front()->staticParamMap.Size();
    }
  }
  else
  {
    if (parent.isValid())
    {
      auto row = static_cast<std::size_t>(parent.row());
      assert(row < this->m_Fits.size());
      return this->m_Fits[row]->GetParameters().size() + this->m_Fits[row]->staticParamMap.Size();
    }
    else
    {
      return this->m_Fits.size();
    }
  }
}

std::size_t
QmitkFitParameterModel::
getBookmarksCount() const
{
  if (m_Bookmarks.IsNotNull())
  {
    return m_Bookmarks->GetSize();
  }
  return 0;
}

int
QmitkFitParameterModel::
columnCount(const QModelIndex&) const
{
  return 3 + this->getBookmarksCount();
}


/** Helper function returns the name of the static parameter indicates by the index.
 If the index does not indicate a static parameter an empty string will be returned.*/
std::string GetStaticParameterName(const mitk::modelFit::ModelFitInfo* currentFit, const QModelIndex& index)
{
  const auto paramSize = static_cast<int>(currentFit->GetParameters().size());

  std::string staticParamName;

  if (index.row() >= paramSize)
  {
    int pos = paramSize;
    for (const auto& iter : currentFit->staticParamMap)
    {
      if (pos == index.row())
      {
        staticParamName = iter.first;
        break;
      }
      ++pos;
    }
  }

  return staticParamName;
}

QVariant
QmitkFitParameterModel::
data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  QVariant result;
  if (!index.parent().isValid() && !this->hasSingleFit())
  { //we need the fit names
    if (index.row() < static_cast<int>(m_Fits.size()) && index.column() == 0)
    {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
        result = QVariant(QString::fromStdString(m_Fits[index.row()]->fitName)+QString("(") + QString::fromStdString(m_Fits[index.row()]->uid) + QString(")"));
      }
      else if (role == Qt::ToolTipRole)
      {
        result = QVariant("Name (UID) of the fit.");
      }
    }
  }
  else
  { // realy want to get the values of the current fit
    const mitk::modelFit::ModelFitInfo* currentFit = nullptr;

    if (this->hasSingleFit() && !index.parent().isValid())
    {
      currentFit = m_Fits.front();
    }
    else if (index.parent().isValid() && index.parent().row() < static_cast<int>(m_Fits.size()))
    {
      currentFit = m_Fits[index.parent().row()];
    }

    if (currentFit)
    {
      const auto paramSize = static_cast<int>(currentFit->GetParameters().size());
      const auto staticParamSize = static_cast<int>(currentFit->staticParamMap.Size());

      if (index.row() < paramSize + staticParamSize)
      {
        std::string staticParamName = GetStaticParameterName(currentFit, index);

        switch (index.column())
        {
        case 0:
          if (role == Qt::DisplayRole || role == Qt::EditRole)
          {
            if (index.row() < paramSize)
            {
              const auto& param = currentFit->GetParameters()[index.row()];
              result = QVariant(QString::fromStdString(param->name));
            }
            else
            {
              result = QVariant(QString::fromStdString(staticParamName));
            }
          }
          else if (role == Qt::ToolTipRole)
          {
            result = QVariant("Name of the parameter.");
          }
          break;

        case 1:
          if (role == Qt::DisplayRole || role == Qt::EditRole)
          {
            if (index.row() < paramSize)
            {
              const auto& param = currentFit->GetParameters()[index.row()];
              std::string paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER();

              if (param->type == mitk::modelFit::Parameter::DerivedType)
              {
                paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER();
              }
              else if (param->type == mitk::modelFit::Parameter::CriterionType)
              {
                paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_CRITERION();
              }
              else if (param->type == mitk::modelFit::Parameter::EvaluationType)
              {
                paramType = mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER();
              }

              result = QVariant(QString::fromStdString(paramType));
            }
            else
            {
              result = QVariant("static");
            }
          }
          else if (role == Qt::ToolTipRole)
          {
            result = QVariant("Type of the parameter.");
          }
          break;

        default:
          if (index.column() - 2 < static_cast<int>(this->getBookmarksCount()+1))
          {
            mitk::Point3D pos = m_CurrentPos;
            if (index.column() > 2)
            {
              pos = m_Bookmarks->GetPoint(index.column() - 3);
            }

            if (role == Qt::DisplayRole || role == Qt::EditRole)
            {
              if (index.row() < paramSize)
              {
                auto value = mitk::ReadVoxel(currentFit->GetParameters()[index.row()]->image, pos);
                result = QVariant(QString::number(value));
              }
              else
              {
                auto value = currentFit->staticParamMap.Get(staticParamName).front();
                result = QVariant(QString::number(value));
              }
            }
            else if (role == Qt::ToolTipRole)
            {
              result = QVariant("Value of a (static) fit parameter");
            }
          }
          break;
        }
      }
    }
  }

  return result;
}

Qt::ItemFlags
QmitkFitParameterModel::
flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  return flags;
}

QVariant
QmitkFitParameterModel::
headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) &&
      (Qt::Horizontal == orientation))
  {
    if (section == 0)
    {
      return QVariant("Name");
    }
    else if (section == 1)
    {
      return QVariant("Type");
    }
    else if (section == 2)
    {
      return QVariant("Value");
    }
    else if (section - 3 < static_cast<int>(this->getBookmarksCount()))
    {
      const auto & pos = m_Bookmarks->GetPoint(section - 3);
      std::ostringstream strm;
      strm.imbue(std::locale("C"));
      strm << std::setprecision(3) << "Value @ Pos " << section -3 << " (" << pos[0] << "|" << pos[1] << "|" << pos[2] << ")";
      return QVariant(QString::fromStdString(strm.str()));
    }
  }

  return QVariant();
}

bool
QmitkFitParameterModel::
setData(const QModelIndex&, const QVariant&, int)
{
  return false;
};
