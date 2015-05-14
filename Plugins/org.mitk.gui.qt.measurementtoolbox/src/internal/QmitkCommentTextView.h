#pragma once

#include <qdialog.h>

#include "ui_QmitkCommentTextView.h"

class QmitkCommentTextView : public QDialog
{
	Q_OBJECT

public:

	QmitkCommentTextView();
	~QmitkCommentTextView();

	QString getText();
	void show();

private:

	Ui::QmitkCommentTextView m_gui;
	QString m_commentText;

private slots:

	void done();
};

