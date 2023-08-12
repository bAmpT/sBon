#ifndef EDITPRODUCTDIALOG_H
#define EDITPRODUCTDIALOG_H

#include <QDialog>
#include <QtWidgets>
#include "bontablemodel.h"

class EditProductDialog : public QDialog
{
    Q_OBJECT
public:
    EditProductDialog(QWidget *parent = nullptr);
    //EditProductDialog(QWidget *parent, Product *product);
    void loadProduct(Product *product);
    void saveProduct();
private:
    Product *product;
    QLabel *titleLabel;
    QLabel *priceLabel;
    QLineEdit *titleEdit;
    QLineEdit *priceEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *saveButton;
    QPushButton *cancelButton;
};

#endif // EDITPRODUCTDIALOG_H
