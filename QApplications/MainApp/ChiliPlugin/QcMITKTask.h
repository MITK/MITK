#ifndef QCMITKTASK_H
#define QCMITKTASK_H

#include <chili/task.h>

class  QcMITKTask: public QcTask {

public:

    QcMITKTask (const char* xpm[], QWidget* parent, const char* name);
    ~QcMITKTask ();

    virtual void showEvent(QShowEvent *se);
    virtual void hideEvent(QHideEvent *se);
};



#endif

