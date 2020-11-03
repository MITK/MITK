/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QColor>
#include <QSize>

#include "QmitkIsoDoseLevelSetModel.h"
#include "mitkRTUIConstants.h"

QmitkIsoDoseLevelSetModel::QmitkIsoDoseLevelSetModel(QObject *parent)
  : QAbstractTableModel(parent),
    m_showAbsoluteDose(false),
    m_visibilityEditOnly(false),
    m_referenceDose(mitk::RTUIConstants::DEFAULT_REFERENCE_DOSE_VALUE),
    m_modified(false)
{
  m_DoseSet = mitk::IsoDoseLevelSet::New();
}

void
  QmitkIsoDoseLevelSetModel::
  setIsoDoseLevelSet(mitk::IsoDoseLevelSet *pSet)
{
  if (pSet)
  {
    emit beginResetModel();

    m_DoseSet = pSet;
    m_modified = false;

    emit endResetModel();
  }
};

int
  QmitkIsoDoseLevelSetModel::
  rowCount(const QModelIndex &parent) const
{
  if(parent.isValid())
  {
    return 0;
  }

  return m_DoseSet->Size();
}

int
  QmitkIsoDoseLevelSetModel::
  columnCount(const QModelIndex &parent) const
{
  if(parent.isValid())
    return 0;

  return 4;
}

QVariant
  QmitkIsoDoseLevelSetModel::
  data(const QModelIndex &index, int role) const
{
  if(!index.isValid())
    return QVariant();

  QVariant result;

  if (static_cast<unsigned int>(index.row()) < m_DoseSet->Size())
  {
    const mitk::IsoDoseLevel& level = m_DoseSet->GetIsoDoseLevel(static_cast<mitk::IsoDoseLevelSet::IsoLevelIndexType>(index.row()));

    switch(index.column())
    {
    case 0:
      if(role == Qt::EditRole || role == Qt::DecorationRole)
      {
        QColor color;
        color.setRgbF(level.GetColor().GetRed(),level.GetColor().GetGreen(),level.GetColor().GetBlue());
        result = QVariant(color);
      }
      else if (role == Qt::ToolTipRole)
      {
        result = QVariant("Color of the iso dose level.");
      }
      break;
    case 1:
      if(role == Qt::DisplayRole)
      {
        if (this->m_showAbsoluteDose)
        {
          result = QVariant(QString::number(level.GetDoseValue()*this->m_referenceDose)+QString(" Gy"));
        }
        else
        {
          result = QVariant(QString::number(level.GetDoseValue()*100)+QString(" %"));
        }
      }
      else if(role == Qt::EditRole)
      {
        if (this->m_showAbsoluteDose)
        {
          result = QVariant(level.GetDoseValue()*this->m_referenceDose);
        }
        else
        {
          result = QVariant(level.GetDoseValue()*100);
        }
      }
      else if (role == Qt::ToolTipRole)
      {
        result = QVariant("Minimum dose value of this level / Value of the iso line.");
      }
      else if (role == Qt::UserRole+1)
      {
        result = QVariant(this->m_showAbsoluteDose);
      }
      break;
    case 2:
      if(role == Qt::DisplayRole || role == Qt::EditRole)
      {
        result = QVariant(level.GetVisibleIsoLine());
      }
      else if (role == Qt::ToolTipRole)
      {
        result = QVariant("Show isoline for this dose level.");
      }
      break;
    case 3:
      if(role == Qt::DisplayRole || role == Qt::EditRole)
      {
        result = QVariant(level.GetVisibleColorWash());
      }
      else if (role == Qt::ToolTipRole)
      {
        result = QVariant("Show colorwash for this dose level.");
      }
      break;
    }
  }

  return result;
}

Qt::ItemFlags
  QmitkIsoDoseLevelSetModel::
  flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if (static_cast<unsigned int>(index.row()) < m_DoseSet->Size())
  {
    if (index.column() < 4)
    {
      if ((index.column() > 1) || (index.column() >= 0 && !this->m_visibilityEditOnly))
      {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
      }
      else if (index.column() >= 0 && this->m_visibilityEditOnly)
      {
        flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
      }
    }
  }

  return flags;
}

QVariant
  QmitkIsoDoseLevelSetModel::
  headerData(int section, Qt::Orientation orientation, int role) const
{
  if( (Qt::DisplayRole == role) &&
    (Qt::Horizontal == orientation))
  {
    if (section==0)
    {
      return QVariant("Color");
    }
    else if (section==1)
    {
      if (m_showAbsoluteDose)
      {
        return QVariant("Dose [Gy]");
      }
      else
      {
        return QVariant("Dose [%]");
      }
    }
    else if (section==2)
    {
      return QVariant("IsoLines");
    }
    else if (section==3)
    {
      return QVariant("ColorWash");
    }
  }
  return QVariant();
}

bool
  QmitkIsoDoseLevelSetModel::
  setData(const QModelIndex &index, const QVariant &value, int role)
{
  if(!index.isValid() || (m_DoseSet->Size() <= static_cast<unsigned int>(index.row())) || (index.column()>3))
  {
    return false;
  }

  if(Qt::EditRole == role)
  {
    const mitk::IsoDoseLevel& level = m_DoseSet->GetIsoDoseLevel(static_cast<mitk::IsoDoseLevelSet::IsoLevelIndexType>(index.row()));
    mitk::IsoDoseLevel::Pointer pNewLevel = level.Clone();
    switch(index.column())
    {
    case 0:
      {
        QColor val = value.value<QColor>();
        mitk::IsoDoseLevel::ColorType color;
        color.SetRed(val.redF());
        color.SetGreen(val.greenF());
        color.SetBlue(val.blueF());
        pNewLevel->SetColor(color);
        emit dataChanged(index,index);
        break;
      }
    case 1:
      if (this->m_showAbsoluteDose)
      {
        pNewLevel->SetDoseValue(value.toDouble()/this->m_referenceDose);
      }
      else
      {
        pNewLevel->SetDoseValue(value.toDouble()/100.0);
      }
      emit dataChanged(index,index);
      break;
    case 2:
      pNewLevel->SetVisibleIsoLine(value.toBool());
      emit dataChanged(index,index);
      break;
    case 3:
      pNewLevel->SetVisibleColorWash(value.toBool());
      emit dataChanged(index,index);
      break;
    }

    emit beginResetModel();

    m_DoseSet->DeleteIsoDoseLevel(static_cast<mitk::IsoDoseLevelSet::IsoLevelIndexType>(index.row()));
    m_DoseSet->SetIsoDoseLevel(pNewLevel);

    m_modified = true;

    emit endResetModel();

    return true;
  }

  return false;
};

void QmitkIsoDoseLevelSetModel::
  setReferenceDose(double newReferenceDose)
{
  if (newReferenceDose<= 0)
  {
    mitkThrow() << "Error. Passed prescribed dose is negative or equals 0.";
  }

  if (newReferenceDose != m_referenceDose)
  {
    this->m_referenceDose = newReferenceDose;
    if(m_showAbsoluteDose)
    {
      emit beginResetModel();
      emit endResetModel();
    }
  }
};

void QmitkIsoDoseLevelSetModel::
  setShowAbsoluteDose(bool showAbsoluteDose)

{
  if (showAbsoluteDose != m_showAbsoluteDose)
  {
    emit beginResetModel();
    this->m_showAbsoluteDose = showAbsoluteDose;
    emit endResetModel();
  }
};

void QmitkIsoDoseLevelSetModel::setVisibilityEditOnly(bool onlyVisibility)
{
  if (onlyVisibility != m_visibilityEditOnly)
  {
    emit beginResetModel();
    this->m_visibilityEditOnly = onlyVisibility;
    emit endResetModel();
  }
};

bool
  QmitkIsoDoseLevelSetModel::
  getShowAbsoluteDose() const
{
  return this->m_showAbsoluteDose;
};

mitk::DoseValueAbs
  QmitkIsoDoseLevelSetModel::
  getReferenceDose() const
{
  return this->m_referenceDose;
};

bool
  QmitkIsoDoseLevelSetModel::
  getVisibilityEditOnly() const
{
  return this->m_visibilityEditOnly;
};

void QmitkIsoDoseLevelSetModel::switchVisibilityIsoLines(bool activate)
{
  emit beginResetModel();

  for (mitk::IsoDoseLevelSet::IsoLevelIndexType pos = 0; pos < m_DoseSet->Size(); ++pos)
  {
    mitk::IsoDoseLevel::Pointer pNewLevel = m_DoseSet->GetIsoDoseLevel(pos).Clone();
    pNewLevel->SetVisibleIsoLine(activate);
    m_DoseSet->SetIsoDoseLevel(pNewLevel);
  }

  m_modified = true;

  emit endResetModel();
}

void QmitkIsoDoseLevelSetModel::switchVisibilityColorWash(bool activate)
{
  emit beginResetModel();

  for (mitk::IsoDoseLevelSet::IsoLevelIndexType pos = 0; pos < m_DoseSet->Size(); ++pos)
  {
    mitk::IsoDoseLevel::Pointer pNewLevel = m_DoseSet->GetIsoDoseLevel(pos).Clone();
    pNewLevel->SetVisibleColorWash(activate);
    m_DoseSet->SetIsoDoseLevel(pNewLevel);
  }

  m_modified = true;

  emit endResetModel();
}

void QmitkIsoDoseLevelSetModel::invertVisibilityIsoLines()
{
  emit beginResetModel();

  for (mitk::IsoDoseLevelSet::IsoLevelIndexType pos = 0; pos < m_DoseSet->Size(); ++pos)
  {
    mitk::IsoDoseLevel::Pointer pNewLevel = m_DoseSet->GetIsoDoseLevel(pos).Clone();
    pNewLevel->SetVisibleIsoLine(!pNewLevel->GetVisibleIsoLine());
    m_DoseSet->SetIsoDoseLevel(pNewLevel);
  }

  m_modified = true;

  emit endResetModel();
}

void QmitkIsoDoseLevelSetModel::invertVisibilityColorWash()
{
  emit beginResetModel();

  for (mitk::IsoDoseLevelSet::IsoLevelIndexType pos = 0; pos < m_DoseSet->Size(); ++pos)
  {
    mitk::IsoDoseLevel::Pointer pNewLevel = m_DoseSet->GetIsoDoseLevel(pos).Clone();
    pNewLevel->SetVisibleColorWash(!pNewLevel->GetVisibleColorWash());
    m_DoseSet->SetIsoDoseLevel(pNewLevel);
  }

  m_modified = true;

  emit endResetModel();
}

void QmitkIsoDoseLevelSetModel::swapVisibility()
{
  emit beginResetModel();

  for (mitk::IsoDoseLevelSet::IsoLevelIndexType pos = 0; pos < m_DoseSet->Size(); ++pos)
  {
    mitk::IsoDoseLevel::Pointer pNewLevel = m_DoseSet->GetIsoDoseLevel(pos).Clone();
    bool colorWash = pNewLevel->GetVisibleColorWash();
    pNewLevel->SetVisibleColorWash(pNewLevel->GetVisibleIsoLine());
    pNewLevel->SetVisibleIsoLine(colorWash);
    m_DoseSet->SetIsoDoseLevel(pNewLevel);
  }

  m_modified = true;

  emit endResetModel();
}

void QmitkIsoDoseLevelSetModel::addLevel()
{
  mitk::IsoDoseLevel::DoseValueType doseVal = 0.01;

  if (m_DoseSet->Size()>0)
  {
    doseVal = m_DoseSet->GetIsoDoseLevel(m_DoseSet->Size()-1).GetDoseValue()+0.01;
  }

  mitk::IsoDoseLevel::Pointer pNewLevel = mitk::IsoDoseLevel::New(doseVal, mitk::IsoDoseLevel::ColorType(), true, true);

  emit beginResetModel();
  m_DoseSet->SetIsoDoseLevel(pNewLevel);
  m_modified = true;
  emit endResetModel();
}

void QmitkIsoDoseLevelSetModel::deleteLevel(const QModelIndex &index)
{
  if(!index.isValid() || (m_DoseSet->Size() <= static_cast<unsigned int>(index.row())) || (index.column()>3))
  {
    return;
  }

  emit beginResetModel();
  m_DoseSet->DeleteIsoDoseLevel(static_cast<mitk::IsoDoseLevelSet::IsoLevelIndexType>(index.row()));
  m_modified = true;
  emit endResetModel();
}

bool QmitkIsoDoseLevelSetModel::isModified()
{
  return m_modified;
}
