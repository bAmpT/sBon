#include "productdelegate.h"

#include <QtWidgets>

ProductDelegate::ProductDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{
}

bool ProductDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {
        //QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        return false;
    }

    //if (index.column() != 5)
    return QSqlRelationalDelegate::editorEvent(event, model, option, index);
}
