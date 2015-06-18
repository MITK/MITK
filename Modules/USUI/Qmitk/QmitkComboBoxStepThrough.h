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

#ifndef QmitkComboBoxStepThrough_H
#define QmitkComboBoxStepThrough_H

#include <QComboBox>

/**
  * \brief Extension of QComboBox which offers stepping through the values.
  *
  * Slots added for setting index to the next position and the previous position.
  * Signals added for signaling if the begin or the end of the values is reached.
  */
class QmitkComboBoxStepThrough : public QComboBox
{
  Q_OBJECT

signals:
  /**
    * True if the end of the available values is reached. False if index was
    * the last possible index and is now another index.
    */
  void SignalReachedEnd(bool);

  /**
    * True if the begin of the available values is reached. False if index was
    * the first possible index and is now another index.
    */
  void SignalReachedBegin(bool);

public slots:
  /**
    * Set index of the combo box to previous index. If current index is already
    * the first index, nothing is done.
    */
  void OnSetPreviousIndex();

  /**
    * Set index of the combo box to next index. If current index is already
    * the last index, nothing is done.
    */
  void OnSetNextIndex();

private slots:
  /**
    * Emits SignalReachedEnd and SignalReachedBegin signals if conditions are met.
    */
  void OnCurrentIndexChanged(int);

public:
  QmitkComboBoxStepThrough(QWidget* parent = nullptr);
  ~QmitkComboBoxStepThrough();

  // --> Overwritten superclass methods
  void addItem( const QString & text, const QVariant & userData = QVariant() );
  void addItem( const QIcon & icon, const QString & text, const QVariant & userData = QVariant() );
  void addItems( const QStringList & texts );

  void insertItem( int index, const QString & text, const QVariant & userData = QVariant() );
  void insertItem( int index, const QIcon & icon, const QString & text, const QVariant & userData = QVariant() );
  void insertItems( int index, const QStringList & list );
  // Overwritten superclass methods <--

private:
  int m_LastMaxIndex;
  int m_LastIndex;
};

#endif // QmitkComboBoxStepThrough_H