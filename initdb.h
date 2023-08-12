#ifndef INITDB_H
#define INITDB_H

#include <QtSql>

const auto BONS_SQL = QLatin1String(R"(
    create table Bons(id integer primary key, bonid integer, amount integer, title varchar, price float, tax float, billed_at text)
    )");

const auto INSERT_BON_SQL = QLatin1String(R"(
    insert into Bons(bonid, amount, title, price, tax, billed_at)
                     values(?, ?, ?, ?, ?, ?)
    )");

const auto PRODUCTS_SQL = QLatin1String(R"(
    create table Products(id integer primary key, title varchar, price float)
    )");

const auto INSERT_PRODUCT_SQL = QLatin1String(R"(
    insert into Products(title, price)
                         values(?, ?)
    )");

void addProduct(QSqlQuery &q, const QString &title, float price)
{
    if (!q.prepare(INSERT_PRODUCT_SQL)) {
        qDebug() << q.lastError();
        return;
    }
    q.addBindValue(title);
    q.addBindValue(price);
    q.exec();
}

void addBon(QSqlQuery &q, int bonid, int amount, const QString &title, float price, float tax, const QString &billed_at)
{
    if (!q.prepare(INSERT_BON_SQL)) {
        qDebug() << q.lastError();
        return;
    }
    q.addBindValue(bonid);
    q.addBindValue(amount);
    q.addBindValue(title);
    q.addBindValue(price);
    q.addBindValue(tax);
    q.addBindValue(billed_at);
    q.exec();
}

QSqlError initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/tmp/sBon_DB_decrypt.db");
    if (!QSqlDatabase::isDriverAvailable("SQLITECIPHER"))
        return QSqlError("Secure sql driver not available!");

    if (!db.open())
        return db.lastError();

    QString dbsecret("password");

    QSqlQuery q;
    if(!q.exec(QString("PRAGMA key='%1';").arg(dbsecret)))
        return QSqlError("Database could not be decrypted!");

    QStringList tables = db.tables();
    if (!tables.contains("Products", Qt::CaseInsensitive))
    {
        QSqlQuery q;
        if (!q.exec(PRODUCTS_SQL))
            return q.lastError();

        addProduct(q, QStringLiteral("Rindfleischroulade mit Bohnen"), 12.5);
        addProduct(q, QStringLiteral("Rotkohl mit Hirschrücken"), 18.99);
    }

    if (!tables.contains("Bons", Qt::CaseInsensitive))
    {
        QSqlQuery q;
        if (!q.exec(BONS_SQL))
            return q.lastError();
    }

    return QSqlError();
}
#endif // INITDB_H
