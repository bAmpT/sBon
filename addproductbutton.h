#ifndef ADDPRODUCTBUTTON_H
#define ADDPRODUCTBUTTON_H

#include "bontablemodel.h"
#include <QtWidgets>

class AddProductButton: public QPushButton
{
    Q_OBJECT

signals:
    void updatedProductButton(int buttonId, Product *product, bool save = true);

public:
    AddProductButton(QWidget *parent = nullptr);

    Product getProduct();
    void setProduct(Product *product, bool update = true);

    QString getLabelText();
    void setLabelText(const QString &text);

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    Product *product;
    QLabel *textLabel;
};

#endif // ADDPRODUCTBUTTON_H
