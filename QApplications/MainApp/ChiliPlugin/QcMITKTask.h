#ifndef QCMITKTASK_H
#define QCMITKTASK_H

#include <chili/task.h>

class  QcMITKTask: public QcTask {

public:

    QcMITKTask (const char* xpm[], QWidget* parent, const char* name);
    ~QcMITKTask ();

    virtual void showEvent(QShowEvent *se);
    virtual void hideEvent(QHideEvent *se);

    virtual void VisibleOff();
    virtual void VisibleOn();
    virtual bool IsVisible();

private:
    bool m_Visible;
};



#endif

