#include "addproductbutton.h"
#include "editproductdialog.h"
#include "bontablemodel.h"

#include <QtWidgets>

AddProductButton::AddProductButton(QWidget *parent)
    : QPushButton(parent)
{
    this->setAcceptDrops(true);

    this->textLabel = new QLabel(this);
    this->textLabel->setWordWrap(true);
    this->textLabel->setAlignment(Qt::AlignCenter);
    this->textLabel->setMouseTracking(false);
    this->textLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    this->setLayout(new QVBoxLayout(this));
    this->layout()->addWidget(this->textLabel);

    this->product = new Product();
}

Product AddProductButton::getProduct()
{
    return *this->product;
}

void AddProductButton::setProduct(Product *product, bool update)
{
    qDebug() << QString("Updating button: %1").arg(this->objectName());
    this->product = product;
    if (update)
        emit updatedProductButton(this->objectName().split("_")[1].toInt(), product);
}

QString AddProductButton::getLabelText()
{
    return this->textLabel->text();
}

void AddProductButton::setLabelText(const QString &text)
{
    this->textLabel->setText(text);
    this->setText("");
}

void AddProductButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // handle right mouse button here
        qDebug() << QString("Right button pressed!");
        QMenu *menu = new QMenu(this);
        this->setMenu(menu);

        QAction *newAction = new QAction("Eintrag bearbeiten", this);
        connect(newAction, &QAction::triggered,
            [this]() {
            qDebug("Menu: Bearbeiten Action!");
            EditProductDialog *dialog = new EditProductDialog();
            dialog->loadProduct(this->product);
            dialog->exec();

            setProduct(this->product);

            /*
            QDialog dialog(this);
            // Use a layout allowing to have a label next to each field
            QFormLayout form(&dialog);

            // Add some text above the fields
            form.addRow(new QLabel("The question ?"));

            // Add the lineEdits with their respective labels
            QList<QLineEdit *> fields;
            for(int i = 0; i < 4; ++i) {
                QLineEdit *lineEdit = new QLineEdit(&dialog);
                QString label = QString("Value %1").arg(i + 1);
                form.addRow(label, lineEdit);

                fields << lineEdit;
            }

            // Add some standard buttons (Cancel/Ok) at the bottom of the dialog
            QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                       Qt::Horizontal, &dialog);
            form.addRow(&buttonBox);
            QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
            QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

            // Show the dialog as modal
            if (dialog.exec() == QDialog::Accepted) {
                // If the user didn't dismiss the dialog, do something with the fields
                foreach(QLineEdit * lineEdit, fields) {
                    qDebug() << lineEdit->text();
                }
            }
            */
        });
        menu->addAction(newAction);

        this->showMenu();
        this->setMenu(nullptr);

    } else if (event->button() == Qt::LeftButton) {
        if (event->modifiers() == Qt::ShiftModifier) {
            // Ask for product from product table
            emit updatedProductButton(this->objectName().split("_")[1].toInt(), nullptr);
            return;
        }
    }
    // pass on other buttons to base class
    QAbstractButton::mousePressEvent(event);
}

void AddProductButton::dropEvent(QDropEvent *event)
{
    qDebug("Drop Event triggered!");

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasText()) {
        qDebug() << mimeData->text();
        setLabelText(mimeData->text());
        //setTextFormat(Qt::PlainText);
    }
    if (mimeData->hasFormat("application/product.struct")) {
        QByteArray data = mimeData->data("application/product.struct");
        QDataStream stream(&data, QIODevice::ReadOnly);

        Product *product = new Product();
        stream >> *product;
        this->product = product;

        qDebug() << product->title << product->price;
        setProduct(product);
    }
    event->acceptProposedAction();
}

void AddProductButton::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug("Drag Enter Event triggered!");
    // TODO: Check product id and name
    event->acceptProposedAction();
}
