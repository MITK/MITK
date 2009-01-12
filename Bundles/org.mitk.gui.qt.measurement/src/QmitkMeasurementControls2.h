#ifndef QMITKMEASUREMENTCONTROLS_H
#define QMITKMEASUREMENTCONTROLS_H

#include <qvariant.h>


#include <Qt3Support/Q3MimeSourceFactory>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QmitkMeasurementControls
{
public:
    QVBoxLayout *vboxLayout;
    QLabel *textLabel1_3_2;
    QPushButton *pbDistance;
    QPushButton *pbAngle;
    QPushButton *pbPath;
    QLabel *textLabel1_3;
    QSpacerItem *spacer2;

    void setupUi(QWidget *QmitkMeasurementControls)
    {
    if (QmitkMeasurementControls->objectName().isEmpty())
        QmitkMeasurementControls->setObjectName(QString::fromUtf8("QmitkMeasurementControls"));
    QmitkMeasurementControls->resize(227, 605);
    QmitkMeasurementControls->setMinimumSize(QSize(0, 0));
    vboxLayout = new QVBoxLayout(QmitkMeasurementControls);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(11);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    textLabel1_3_2 = new QLabel(QmitkMeasurementControls);
    textLabel1_3_2->setObjectName(QString::fromUtf8("textLabel1_3_2"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(1));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(textLabel1_3_2->sizePolicy().hasHeightForWidth());
    textLabel1_3_2->setSizePolicy(sizePolicy);
    textLabel1_3_2->setMinimumSize(QSize(150, 25));
    textLabel1_3_2->setFrameShape(QFrame::NoFrame);
    textLabel1_3_2->setFrameShadow(QFrame::Plain);
    textLabel1_3_2->setLineWidth(1);
    textLabel1_3_2->setAlignment(Qt::AlignVCenter);
    textLabel1_3_2->setWordWrap(false);

    vboxLayout->addWidget(textLabel1_3_2);

    pbDistance = new QPushButton(QmitkMeasurementControls);
    pbDistance->setObjectName(QString::fromUtf8("pbDistance"));
    pbDistance->setCheckable(false);
    pbDistance->setFlat(false);

    vboxLayout->addWidget(pbDistance);

    pbAngle = new QPushButton(QmitkMeasurementControls);
    pbAngle->setObjectName(QString::fromUtf8("pbAngle"));
    pbAngle->setCheckable(false);

    vboxLayout->addWidget(pbAngle);

    pbPath = new QPushButton(QmitkMeasurementControls);
    pbPath->setObjectName(QString::fromUtf8("pbPath"));
    pbPath->setCursor(QCursor(static_cast<Qt::CursorShape>(0)));
    pbPath->setAcceptDrops(false);
    pbPath->setCheckable(false);
    pbPath->setChecked(false);
    pbPath->setAutoRepeat(false);
    pbPath->setAutoDefault(false);
    pbPath->setDefault(false);
    pbPath->setFlat(false);

    vboxLayout->addWidget(pbPath);

    textLabel1_3 = new QLabel(QmitkMeasurementControls);
    textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));
    sizePolicy.setHeightForWidth(textLabel1_3->sizePolicy().hasHeightForWidth());
    textLabel1_3->setSizePolicy(sizePolicy);
    textLabel1_3->setMinimumSize(QSize(150, 65));
    textLabel1_3->setFrameShape(QFrame::NoFrame);
    textLabel1_3->setFrameShadow(QFrame::Plain);
    textLabel1_3->setLineWidth(1);
    textLabel1_3->setAlignment(Qt::AlignVCenter);
    textLabel1_3->setWordWrap(false);

    vboxLayout->addWidget(textLabel1_3);

    spacer2 = new QSpacerItem(20, 390, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacer2);


    retranslateUi(QmitkMeasurementControls);

    QMetaObject::connectSlotsByName(QmitkMeasurementControls);
    } // setupUi

    void retranslateUi(QWidget *QmitkMeasurementControls)
    {
    QmitkMeasurementControls->setWindowTitle(QApplication::translate("QmitkMeasurementControls", "Measurement", 0, QApplication::UnicodeUTF8));
    textLabel1_3_2->setText(QApplication::translate("QmitkMeasurementControls", "<p align=\"center\">Select one option</p>", 0, QApplication::UnicodeUTF8));
    pbDistance->setText(QApplication::translate("QmitkMeasurementControls", "Distance", 0, QApplication::UnicodeUTF8));

#ifndef QT_NO_TOOLTIP
    pbDistance->setProperty("toolTip", QVariant(QApplication::translate("QmitkMeasurementControls", "calculates the distance between two points", 0, QApplication::UnicodeUTF8)));
#endif // QT_NO_TOOLTIP

    pbAngle->setText(QApplication::translate("QmitkMeasurementControls", "Angle", 0, QApplication::UnicodeUTF8));

#ifndef QT_NO_TOOLTIP
    pbAngle->setProperty("toolTip", QVariant(QApplication::translate("QmitkMeasurementControls", "calculates the angle between two lines", 0, QApplication::UnicodeUTF8)));
#endif // QT_NO_TOOLTIP

    pbPath->setText(QApplication::translate("QmitkMeasurementControls", "Path", 0, QApplication::UnicodeUTF8));

#ifndef QT_NO_TOOLTIP
    pbPath->setProperty("toolTip", QVariant(QApplication::translate("QmitkMeasurementControls", "calculates the distance between two or more points", 0, QApplication::UnicodeUTF8)));
#endif // QT_NO_TOOLTIP

    textLabel1_3->setText(QApplication::translate("QmitkMeasurementControls", "<p align=\"center\">Hold <i>shift + click</i><br>to insert a point</p>", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(QmitkMeasurementControls);
    } // retranslateUi

};

namespace Ui {
    class QmitkMeasurementControls: public Ui_QmitkMeasurementControls {};
} // namespace Ui

QT_END_NAMESPACE

class QmitkMeasurementControls : public QWidget, public Ui::QmitkMeasurementControls
{
    Q_OBJECT

public:
    QmitkMeasurementControls(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~QmitkMeasurementControls();

protected slots:
    virtual void languageChange();

};

#endif // QMITKMEASUREMENTCONTROLS_H
