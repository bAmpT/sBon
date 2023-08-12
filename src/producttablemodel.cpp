#include "producttablemodel.h"
#include "bontablemodel.h"

#include <QMimeData>
#include <QIODevice>

ProductTableModel::ProductTableModel(QObject *parent)
    : QSqlTableModel(parent)
{
}

Qt::DropActions ProductTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags ProductTableModel::flags(const QModelIndex &index) const
{
     //Qt::ItemFlags defaultFlags = QStringListModel::flags(*index);

     if (index.isValid())
         return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;//| defaultFlags;
     else
         return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsEditable;// | defaultFlags;
}

QMimeData *ProductTableModel::mimeData(const QModelIndexList &indexes) const
{
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    /*foreach (QModelIndex index, indexes) {
         if (index.isValid()) {
             QString text = data(index, Qt::DisplayRole).toString();
             stream << text;
         }
    }*/
    QString text = data(indexes[0], Qt::DisplayRole).toString();
    float price = data(indexes[1], Qt::DisplayRole).toFloat();

    Product product = Product();
    product.title = text;
    product.price = price;
    stream << product;

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(text);
    mimeData->setData("application/product.struct", encodedData);
    return mimeData;
}
