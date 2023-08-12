#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <QtWidgets>
#include <QLocale>
#include <QSettings>

enum TaxType {
    Standard = 2, // 19 %
    Landwirtschaft = 1, // 5 %
    Ohne = 0 // 0 %
};

inline float getTax(TaxType taxType)
{
    switch (taxType) {
    case TaxType::Standard:
        return 19.0;
    case TaxType::Landwirtschaft:
        return 5.0;
    default: return 0.0;
    }
}

inline TaxType getTaxType(float tax)
{
    QSettings s = QSettings();
    if (tax == s.value("StandardTax", 19.0).toFloat()) {
        return TaxType::Standard;
    } else if (tax == s.value("LandwirtschaftTax", 5.0).toFloat()) {
        return TaxType::Landwirtschaft;
    } else {
        return TaxType::Ohne;
    }
}


struct Product
{
    int id;
    int amount;
    QString title;
    float price;
    float tax = 19.0;
    TaxType taxtype = TaxType::Standard;

    bool operator==(const Product &other) const
    {
        return title == other.title && price == other.price;
    }
    QString getPrice()
    {
        return QLocale::system().toString(price, 'f', 2);
    }
    QString getPriceFormatted()
    {
        return QLocale::system().toString(price, 'f', 2) + "€";
    }
};

Q_DECLARE_METATYPE(Product);

inline QDataStream &operator<<(QDataStream &stream, const Product &product)
{
    return stream << product.amount << product.title << product.price << product.taxtype << product.tax;
}

inline QDataStream &operator>>(QDataStream &stream, Product &product)
{
    return stream >> product.amount >> product.title >> product.price >> product.taxtype >> product.tax;
}

#endif // PRODUCTMODEL_H
