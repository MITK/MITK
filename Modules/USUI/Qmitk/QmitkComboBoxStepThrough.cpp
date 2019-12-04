/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkComboBoxStepThrough.h"

QmitkComboBoxStepThrough::QmitkComboBoxStepThrough(QWidget* parent)
  : QComboBox(parent), m_LastMaxIndex(0), m_LastIndex(0)
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged(int)));
}

QmitkComboBoxStepThrough::~QmitkComboBoxStepThrough()
{
}

void QmitkComboBoxStepThrough::OnSetPreviousIndex()
{
  int curIndex = this->currentIndex();

  // increase index if not already at end
  if ( curIndex > 0 )
  {
    this->setCurrentIndex(curIndex-1);
  }
}

void QmitkComboBoxStepThrough::OnSetNextIndex()
{
  int curIndex = this->currentIndex();

  // decrease index if not already at begin
  if ( curIndex < this->count() - 1 )
  {
    this->setCurrentIndex(curIndex+1);
  }
}

void QmitkComboBoxStepThrough::OnCurrentIndexChanged(int newIndex)
{
  // emit begin reached singal if index is zero now or was zero before
  if ( m_LastIndex == 0 && newIndex > 0)
  {
    emit SignalReachedBegin(false);
  }
  else if ( m_LastIndex > 0 && newIndex == 0 )
  {
    emit SignalReachedBegin(true);
  }

  int maxIndex = this->count() - 1;

  // emit end reached signal if index is max index now or was max index before
  if ( (m_LastIndex == maxIndex || m_LastIndex == m_LastMaxIndex) && newIndex < maxIndex)
  {
    emit SignalReachedEnd(false);
  }
  else if ( m_LastIndex < maxIndex && newIndex == maxIndex )
  {
    emit SignalReachedEnd(true);
  }

  m_LastIndex = newIndex;
  m_LastMaxIndex = maxIndex;
}

void QmitkComboBoxStepThrough::addItem ( const QString & text, const QVariant & userData )
{
  QComboBox::addItem(text, userData);

  // make sure that begin and end signals are emitted
  OnCurrentIndexChanged(this->currentIndex());
}

void QmitkComboBoxStepThrough::addItem ( const QIcon & icon, const QString & text, const QVariant & userData )
{
  QComboBox::addItem(icon, text, userData);

  // make sure that begin and end signals are emitted
  OnCurrentIndexChanged(this->currentIndex());
}

void QmitkComboBoxStepThrough::addItems ( const QStringList & texts )
{
   QComboBox::addItems(texts);

   // make sure that begin and end signals are emitted
  OnCurrentIndexChanged(this->currentIndex());
}

void QmitkComboBoxStepThrough::insertItem( int index, const QString & text, const QVariant & userData )
{
  QComboBox::insertItem(index, text, userData);

  // make sure that begin and end signals are emitted
  OnCurrentIndexChanged(this->currentIndex());
}

void QmitkComboBoxStepThrough::insertItem( int index, const QIcon & icon, const QString & text, const QVariant & userData )
{
  QComboBox::insertItem(index, icon, text, userData);

  // make sure that begin and end signals are emitted
  OnCurrentIndexChanged(this->currentIndex());
}

void QmitkComboBoxStepThrough::insertItems( int index, const QStringList & list )
{
  QComboBox::insertItems(index, list);

  // make sure that begin and end signals are emitted
  OnCurrentIndexChanged(this->currentIndex());
}
