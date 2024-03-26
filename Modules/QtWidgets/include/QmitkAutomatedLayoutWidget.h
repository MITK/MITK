/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAutomatedLayoutDialog_h
#define QmitkAutomatedLayoutDialog_h

#include "MitkQtWidgetsExports.h"

#include "ui_QmitkAutomatedLayoutWidget.h"

//#include <QmitkNodeSelectionDialog.h>

#include <qwidget.h>

class MITKQTWIDGETS_EXPORT QmitkAutomatedLayoutWidget : public QWidget
{
	Q_OBJECT

private Q_SLOTS:
	
	void OnSelectDataClicked();
	void OnSetLayoutClicked();
	void OnSelectionDialogClosed();

public:
	QmitkAutomatedLayoutWidget(QWidget* parent = nullptr);

private:

	Ui::QmitkAutomatedLayoutWidget m_Controls;
};


#endif