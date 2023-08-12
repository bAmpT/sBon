#ifndef BONTABLEMODEL_H
#define BONTABLEMODEL_H

#include <QtWidgets>
#include <QAbstractTableModel>
#include <QList>
#include "productmodel.h"

/*Bon AbstractTableModel*/
class BonTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit BonTableModel(QObject *parent = nullptr, int numTables = 1);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    void insertProduct(int amount, Product product);

    float getSum() const;
    bool setTable(int tableId);
    QList<Product> getProducts(int tableId);
    QList<Product> getProducts();

    void saveTable();
    void clearTable();
protected:
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                 int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    //void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
    QList<QList<Product>> tables;
    QList<Product> products;

    int currentTableId = 1;
};

#endif // BONTABLEMODEL_H
