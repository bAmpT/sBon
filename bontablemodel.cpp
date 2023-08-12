#include "bontablemodel.h"

BonTableModel::BonTableModel(QObject *parent, int numTables)
    : QAbstractTableModel(parent)
{
    //QSettings settings("/tmp/sBon.ini", QSettings::NativeFormat);
    QSettings settings = QSettings("/tmp/sBon_tables.ini", QSettings::NativeFormat);

    tables = QList<QList<Product>>();
    products = QList<Product>();

    for (int i = 0; i < numTables; i++) {
        QVariant p = settings.value(QString("tableContents/%1").arg(i));
        products = p.value<QList<Product>>();
        tables.append(products);

    }
    currentTableId = 1;
    if (tables.count() > currentTableId)
        products = tables[currentTableId];
}

int BonTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : products.size();
}

int BonTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 5;
}

QVariant BonTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= products.size() || index.row() < 0)
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto &product = products.at(index.row());

        switch (index.column()) {
            case 0:
                return product.amount;
            case 1:
                return product.title;
            case 2:
                return product.tax;
            case 3:
                return product.price;
            case 4:
                return product.amount * product.price;
            default:
                break;
        }
    }
    if (role == Qt::EditRole) {
        Product product = products.at(index.row());
        switch (index.column()) {
            case 0:
                return product.amount;
            case 1:
                return product.title;
            case 2:
                return QString::number(product.tax, 'f', 2);
            case 3:
                return QString::number(product.price, 'f', 2);
            case 4:
                return product.amount * product.price;
            default:
                break;
        }
    }
    return QVariant();
}

QVariant BonTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Anzahl");
            case 1:
                return tr("Titel");
            case 2:
                return tr("Steuer");
            case 3:
                return tr("Preis");
            case 4:
                return tr("Summe");
            default:
                break;
        }
    }
    return QVariant();
}

bool BonTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        products.insert(position, {0, 1, QString(), 19.0, 0.0});

    endInsertRows();

    return true;
}

bool BonTableModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        products.removeAt(position);

    endRemoveRows();

    //saveTable();
    return true;
}

bool BonTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();
        auto product = products.value(row);

        switch (index.column()) {
            case 0:
                product.amount = value.toInt();
                break;
            case 1:
                product.title = value.toString();
                break;
            case 2:
                product.tax = value.toFloat();
                break;
            case 3:
                product.price = value.toFloat();
                break;
            case 4:
                product.taxtype = TaxType(value.toInt());
                break;
            default:
                return false;
        }
        products.replace(row, product);
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }

    return false;
}

Qt::ItemFlags BonTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() or index.column() == 2)
        return QAbstractTableModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions BonTableModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList BonTableModel::mimeTypes() const
{
    return QStringList{"application/product.struct"};
}

bool BonTableModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                     int row, int column, const QModelIndex &parent) const
{
    if (!data->hasFormat("application/vnd.text.list"))
        return false;

    if (column > 0)
        return false;

    return true;
}

bool BonTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    Q_UNUSED(column);

    if (action == Qt::IgnoreAction)
        return true;

    /*TODO: Enable drop from product table with amount=1*/

    return QAbstractTableModel::dropMimeData(data, action, row, 0, parent);
}


bool BonTableModel::setTable(int tableId)
{
    if (tableId >= 0 && tableId < tables.count()) {
        // Backup products into tables
        tables[currentTableId] = products;

        // Load table products
        removeRows(0, products.count()); // removes products
        insertRows(0, tables[tableId].count()); // inserts empty products

        products = tables[tableId];
        currentTableId = tableId;
    } else {
        return false;
    }
    return true;
}

void BonTableModel::insertProduct(int amount, Product product)
{
    // Add Product to List
    this->insertRows(0, 1, QModelIndex());

    QModelIndex index = this->index(0, 0, QModelIndex());
    this->setData(index, amount, Qt::EditRole);

    index = this->index(0, 1, QModelIndex());
    this->setData(index, product.title, Qt::EditRole);

    index = this->index(0, 2, QModelIndex());
    this->setData(index, product.tax, Qt::EditRole);

    index = this->index(0, 3, QModelIndex());
    this->setData(index, product.price, Qt::EditRole);

    // Add Tax Type (hidden)
    index = this->index(0, 4, QModelIndex());
    this->setData(index, product.taxtype, Qt::EditRole);

    // Save table data
    saveTable();
}

QList<Product> BonTableModel::getProducts(int tableId)
{
    if (tableId > 0 && tableId < this->products.length())
        return this->tables[tableId];
    else
        return QList<Product>();
}

QList<Product> BonTableModel::getProducts()
{
    return this->products;
}

void BonTableModel::saveTable()
{
    QSettings settings = QSettings("/tmp/sBon_tables.ini", QSettings::NativeFormat);

    tables[currentTableId] = products;

    for (int i=0; i<tables.count(); i++) {
        QVariant productsVariant = QVariant::fromValue(this->tables[i]);
        settings.setValue(QString("tableContents/%1").arg(i), productsVariant);
    }
}

void BonTableModel::clearTable()
{
    beginResetModel();
    this->products = QList<Product>();
    this->tables[currentTableId] = this->products;
    endResetModel();
}

float BonTableModel::getSum() const
{
    float sum = 0;
    foreach (Product product, products) {
        sum += product.amount * product.price;
    }
    return sum;
}
