/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void QmitkControlsRightFctLayoutTemplate::init()
{
    //MainSplitter->setResizeMode(MainParent,QSplitter::FollowSizeHint);
    //MainSplitter->setResizeMode(ControlParent,QSplitter::FollowSizeHint);
}


void QmitkControlsRightFctLayoutTemplate::setControlSizeHint(QSize * size)
{
    QValueList<int> sizes;
    int w=10; //MainSplitter->width(); hierin stehen absurde werte. Deshalb funktioniert auch der auto-resize wahrscheinlich nicht!!
    sizes.append(width()-size->width()-w);
    sizes.append(size->width());
    MainSplitter->setSizes(sizes);
}
