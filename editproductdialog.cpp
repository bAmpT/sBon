#include "editproductdialog.h"

EditProductDialog::EditProductDialog(QWidget *parent)
    : QDialog(parent)
{
    titleLabel = new QLabel(tr("Produktname:"));
    titleEdit = new QLineEdit;
    titleLabel->setBuddy(titleEdit);

    priceLabel = new QLabel(tr("Preis:"));
    priceEdit = new QLineEdit;
    priceLabel->setBuddy(priceEdit);

    saveButton = new QPushButton(tr("&Save"));
    saveButton->setDefault(true);
    connect(saveButton, &QAbstractButton::clicked, this, &EditProductDialog::saveProduct);

    cancelButton = new QPushButton(tr("&Cancel"));
    connect(cancelButton, &QAbstractButton::clicked, this, &QDialog::close);

    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(titleLabel);
    topLeftLayout->addWidget(titleEdit);
    QHBoxLayout *bottomLeftLayout = new QHBoxLayout;
    bottomLeftLayout->addWidget(priceLabel);
    bottomLeftLayout->addWidget(priceEdit);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addLayout(bottomLeftLayout);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(leftLayout, 0, 0);
    mainLayout->addWidget(buttonBox, 0, 1);
    mainLayout->setRowStretch(2, 1);

    setLayout(mainLayout);
    setWindowTitle(tr("Produkt bearbeiten"));
}

void EditProductDialog::saveProduct()
{
    // Write changes to the product object
    this->product->title = this->titleEdit->text();
    this->product->price = this->priceEdit->text().toFloat();
    //this->accept();
    this->close();
}

void EditProductDialog::loadProduct(Product *product)
{
    // Load
    this->product = product;
    this->titleEdit->setText(this->product->title);
    this->priceEdit->setText(QString::number(this->product->price));
}
