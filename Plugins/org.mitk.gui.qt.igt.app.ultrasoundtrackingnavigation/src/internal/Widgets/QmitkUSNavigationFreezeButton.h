/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUSNavigationFreezeButton_h
#define QmitkUSNavigationFreezeButton_h

#include <QPushButton>

#include "mitkUSCombinedModality.h"

/**
 * \brief QPushButton for freezing and unfreezing a combined modality.
 * The button already has an icon and a text. On every successfull
 * freeze or unfreeze the signal SignalFrezzed() is emitted. One should
 * listen to this signal rather than to the clicked() signal of the
 * QPushButton as the combined modality may not be freezed after
 * clicked() was emitted.
 */
class QmitkUSNavigationFreezeButton : public QPushButton
{
  Q_OBJECT

signals:
  /**
   * \brief Emitted every time the freeze state of the combined modality changed.
   * True if the combined modality is freezed now, false if it isn't.
   */
  void SignalFreezed(bool);

protected slots:
  void OnButtonClicked(bool checked);
  void OnFreezeButtonToggle();

public:
  explicit QmitkUSNavigationFreezeButton(QWidget* parent = nullptr);
  ~QmitkUSNavigationFreezeButton() override;

  /**
   * \brief Setter for the combined modality to be freezed by this button.
   * An index may be specified for a tracking data output. The combined
   * modality will only be freezed then, if the current tracking data of
   * this output is valid.
   */
  void SetCombinedModality(mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality, int outputIndex = -1);

  /**
   * \brief Try to freeze the combined modality.
   * This does the same as clicking the button while the combined
   * modality isn't freezed. If the combined modality is already
   * freezed this method does nothing.
   */
  void Freeze();

  /**
   * \brief Unfreeze the combined modality.
   * This does the same as clicking the button while the combined
   * modality is freezed. If the combined modality isn't freezed
   * this method does nothing.
   */
  void Unfreeze();

private:
  mitk::AbstractUltrasoundTrackerDevice::Pointer m_CombinedModality;
  int                               m_OutputIndex;
  bool m_FreezeButtonToggle;
};

#endif
