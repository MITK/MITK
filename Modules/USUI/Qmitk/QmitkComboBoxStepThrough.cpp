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

#include "QmitkComboBoxStepThrough.h"

QmitkComboBoxStepThrough::QmitkComboBoxStepThrough(QWidget* parent)
  : QComboBox(parent), m_LastIndex(0)
{
  connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged(int)));
}

QmitkComboBoxStepThrough::~QmitkComboBoxStepThrough()
{

}

void QmitkComboBoxStepThrough::OnSetPreviousIndex()
{
  int curIndex = this->currentIndex();

  if ( curIndex > 0 )
  {
    //if (curIndex == this->count() - 1) { emit SignalReachedEnd(false); }

    this->setCurrentIndex(--curIndex);

    //if (curIndex == 0) { emit SignalReachedBegin(true); }
  }
}

void QmitkComboBoxStepThrough::OnSetNextIndex()
{
  int curIndex = this->currentIndex();

  if ( curIndex < this->count() - 1 )
  {
    //if (curIndex == 0) { emit SignalReachedBegin(false); }

    this->setCurrentIndex(++curIndex);

    //if (curIndex == this->count() -1 ) { emit SignalReachedEnd(true); }
  }

}

void QmitkComboBoxStepThrough::OnCurrentIndexChanged(int newIndex)
{
  if ( m_LastIndex == 0 && newIndex > 0)
  {
    emit SignalReachedBegin(false);
  }
  else if ( m_LastIndex > 0 && newIndex == 0 )
  {
    emit SignalReachedBegin(true);
  }


  int maxIndex = this->count() - 1;

  if ( m_LastIndex == maxIndex && newIndex < maxIndex)
  {
    emit SignalReachedEnd(false);
  }
  else if ( m_LastIndex < maxIndex && newIndex == maxIndex )
  {
    emit SignalReachedEnd(true);
  }

  m_LastIndex = newIndex;
}
