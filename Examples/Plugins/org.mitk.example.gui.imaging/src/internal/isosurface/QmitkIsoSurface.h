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

#ifndef QMITK_ISOSURFACE_H__INCLUDED
#define QMITK_ISOSURFACE_H__INCLUDED

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

  QmitkIsoSurface(QObject *parent=nullptr, const char *name=nullptr);

  virtual ~QmitkIsoSurface();

private:

  /**
   * \brief method for creating the widget containing the application   controls, like sliders, buttons etc.
   */
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /**
   * \brief method for creating the connections of main and control widget
   */
  virtual void CreateConnections();

private slots:

  /*
   * just an example slot for the example TreeNodeSelector widget
   */
  void ImageSelected(const mitk::DataNode* item);

  /**
   * \brief method for creating a surface object
   */
  void CreateSurface();

private:

  /**
   * controls containing sliders for scrolling through the slices
   */
  Ui::QmitkIsoSurfaceControls * m_Controls;

  /**
   * image which is used to create the surface
   */
  mitk::Image* m_MitkImage;

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

#endif // QMITK_ISOSURFACE_H__INCLUDED

