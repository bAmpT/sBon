#ifndef PRODUCTDELEGATE_H
#define PRODUCTDELEGATE_H

#include <QModelIndex>
#include <QSize>
#include <QSqlRelationalDelegate>

class ProductDelegate : public QSqlRelationalDelegate
{
public:
    ProductDelegate(QObject *parent);

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
};
#endif // PRODUCTDELEGATE_H
