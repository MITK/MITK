/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIsoSurface_h
#define QmitkIsoSurface_h

#include "QmitkAbstractView.h"
#include "mitkColorSequenceRainbow.h"
#include "mitkDataStorage.h"
#include "ui_QmitkIsoSurfaceControls.h"

/**
 * \brief IsoSurface
 *
 * \sa QmitkAbstractView
 */
class QmitkIsoSurface : public QmitkAbstractView
{
  Q_OBJECT

public:
  QmitkIsoSurface(QObject *parent = nullptr, const char *name = nullptr);

  ~QmitkIsoSurface() override;

private:
  /**
   * \brief method for creating the widget containing the application   controls, like sliders, buttons etc.
   */
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /**
   * \brief method for creating the connections of main and control widget
   */
  virtual void CreateConnections();

private slots:

  /*
   * just an example slot for the example TreeNodeSelector widget
   */
  void ImageSelected(const mitk::DataNode *item);

  /**
   * \brief method for creating a surface object
   */
  void CreateSurface();

private:
  /**
   * controls containing sliders for scrolling through the slices
   */
  Ui::QmitkIsoSurfaceControls *m_Controls;

  /**
   * image which is used to create the surface
   */
  mitk::Image *m_MitkImage;

  /**
   * read thresholdvalue from GUI and convert it to float
   */
  float getThreshold();

  /**
   *  variable to count Surfaces and give it to name
   */
  int m_SurfaceCounter;

  mitk::ColorSequenceRainbow m_RainbowColor;
};

#endif
