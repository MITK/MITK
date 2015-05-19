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

#ifndef QmitkIsoDoseLevelSetModel_h
#define QmitkIsoDoseLevelSetModel_h

#include <QAbstractTableModel>

#include "mitkIsoDoseLevelCollections.h"

#include "MitkRTUIExports.h"


/*!
\class QmitkIsoDoseLevelSetModel
Model that handles a iso dose level set and allows viewing and editing of its contents.
Please see special delegates (QmitkDoseColorDelegate, QmitkDoseValueDelegate, QmitkDoseVisualStyleDelegate) to
handle visualization and editing in views that work on this model.
\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.
*/
class MITKRTUI_EXPORT QmitkIsoDoseLevelSetModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  QmitkIsoDoseLevelSetModel(QObject *parent = NULL);
  virtual ~QmitkIsoDoseLevelSetModel() {};

  /** Sets the data handled by the model and resets the modified flag*/
  void setIsoDoseLevelSet(mitk::IsoDoseLevelSet *pSet);

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

  bool getShowAbsoluteDose() const;
  mitk::DoseValueAbs getReferenceDose() const;

  bool getVisibilityEditOnly() const;

  void switchVisibilityIsoLines(bool activate);
  void switchVisibilityColorWash(bool activate);
  void invertVisibilityIsoLines();
  void invertVisibilityColorWash();
  void swapVisibility();

  void addLevel();
  void deleteLevel(const QModelIndex &index);

  /**Indicates if the content of the model was modified since the data was set via setIsoDoseLevelSet()*/
  bool isModified();

  public Q_SLOTS:
    /**
    * \brief Slot that can be used to set the prescribed dose.
    */
    void setReferenceDose(double newReferenceDose);
    /**
    * \brief Slot that can be used to adjust whether the dose should be displayed in absolute or relative units.
    */
    void setShowAbsoluteDose(bool showAbsoluteDose);

    /**
    * \brief Slat that can be used to adjust wether the model allows to edit only visibilities (no dose value or color)
    */
    void setVisibilityEditOnly(bool onlyVisibility);

private:
  mitk::IsoDoseLevelSet::Pointer m_DoseSet;

  bool m_showAbsoluteDose;
  bool m_visibilityEditOnly;

  mitk::DoseValueAbs m_referenceDose;

  /** Indicates if the data of the model was modified, since the model was set. */
  bool m_modified;

};

#endif // QmitkIsoDoseLevelSetModel_h

