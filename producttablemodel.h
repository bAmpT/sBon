#ifndef PRODUCTTABLEMODEL_H
#define PRODUCTTABLEMODEL_H

#include <QSqlTableModel>

class ProductTableModel : public QSqlTableModel
{
public:
    explicit ProductTableModel(QObject *parent = nullptr);
private:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    Qt::DropActions supportedDropActions() const;
};

#endif // PRODUCTTABLEMODEL_H
