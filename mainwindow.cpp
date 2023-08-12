#include "mainwindow.h"
#include "producttablemodel.h"
#include "productdelegate.h"
#include "flowlayout.h"
#include "initdb.h"
#include "./ui_mainwindow.h"

#include <QtSql>
#include <QLocale>
#include <QPrintDialog>
#include <QPrinter>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Hide product list
    ui->splitter->setSizes(QList<int>{settings.value("Splitter_1", 1).toInt(),
                                      settings.value("Splitter_2", 0).toInt()});
    // Callback for moving splitter
    QObject::connect(ui->splitter, &QSplitter::splitterMoved, [this]() {
        qDebug("Splitter moved!");
        this->settings.setValue("Splitter_1", this->ui->splitter->sizes().at(0));
        this->settings.setValue("Splitter_2", this->ui->splitter->sizes().at(1));
    });

    // Initialize the database:
    QSqlError err = initDb();
    if (err.type() != QSqlError::NoError) {
        showError(err);
        return;
    }

    /* INIT LOG MODEL */
    logmodel = new QSqlTableModel;
    logmodel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    logmodel->setTable("Bons");
    if (!logmodel->select()) {
        showError(logmodel->lastError());
        return;
    }

    /* INIT PRODUCT MODEL */

    // Create the product data model:
    productmodel = new ProductTableModel(ui->productTable);
    productmodel->setEditStrategy(QSqlTableModel::OnRowChange);
    productmodel->setTable("Products");

    productmodel->setHeaderData(productmodel->fieldIndex("title"),
                         Qt::Horizontal, tr("Title"));
    productmodel->setHeaderData(productmodel->fieldIndex("price"),
                         Qt::Horizontal, tr("Price"));

    // Populate the model:
    if (!productmodel->select()) {
        showError(productmodel->lastError());
        return;
    }

    // Set the model and hide the ID column:
    ui->productTable->setModel(productmodel);
    ui->productTable->setItemDelegate(new ProductDelegate(ui->productTable));
    ui->productTable->setColumnHidden(productmodel->fieldIndex("id"), true);
    ui->productTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->productTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->productTable->setSortingEnabled(true);
    ui->productTable->selectRow(0);

    // Add custom context menu and install event filter
    ui->productTable->viewport()->installEventFilter(this);
    ui->productTable->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->productTable, SIGNAL(customContextMenuRequested(QPoint)),
                     SLOT(customMenuRequested(QPoint)));


    /* INIT BON MODEL*/

    bonmodel = new BonTableModel(ui->bonTable, settings.value("numTables", ui->tableComboBox->count()).toInt());
    bonsearchmodel = new QSortFilterProxyModel(this);
    bonsearchmodel->setSortRole(Qt::DisplayRole);
    bonsearchmodel->setSourceModel(bonmodel);

    ui->bonTable->setModel(bonsearchmodel);
    ui->bonTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->bonTable->setSortingEnabled(false);
    //ui->bonTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->bonTable->setColumnHidden(2, true);
    ui->bonTable->setColumnWidth(0, 50);
    ui->bonTable->setColumnWidth(1, 245);
    ui->bonTable->setColumnWidth(2, 0);
    ui->bonTable->setColumnWidth(3, 50);
    ui->bonTable->setColumnWidth(4, 50);
    ui->bonTable->verticalHeader()->hide();

    ui->bonTable->setDragEnabled(true);
    ui->bonTable->setAcceptDrops(true);
    ui->bonTable->setDropIndicatorShown(true);

    ui->bonTable->viewport()->setAcceptDrops(true);
    ui->bonTable->viewport()->installEventFilter(this);

    QObject::connect(bonmodel, &QAbstractTableModel::dataChanged,
        [this](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
             qDebug("Bontable data changed, updating SumEdit!");
             updateSumLineEdit();
        });

    // Setup Tab Panels
    //ui->tabWidget->saveSettings();
    for (int i=0; i<ui->tabWidget->count(); ++i) {
        ui->tabWidget->removeTab(i);
        delete ui->tabWidget->widget(i);
    }
    ui->tabWidget->loadSettings();

}

void MainWindow::loadSettings()
{
    QSettings settings = QSettings("TamTam-Soft", "sBon_ProductId_Table");

}

void MainWindow::writeSettings()
{
    QSettings settings = QSettings();

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool shouldQuit = true;
    if (shouldQuit) {
        writeSettings();
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MainWindow::showError(const QSqlError &err)
{
    QMessageBox::critical(this, "Unable to initialize Database",
                "Error initializing database: " + err.text());
}

void MainWindow::addProduct()
{
    AddProductButton *pushButton = qobject_cast<AddProductButton*>(sender());
    if (pushButton != NULL) {
        int buttonId = pushButton->objectName().split("_")[1].toInt();
        qDebug("ButtonId: %d", buttonId);

        // Read out the count of products
        QSpinBox *spinBox = getAmountSpinBox(buttonId);
        QSlider *taxSlider = getTaxSlider(buttonId);
        if (spinBox != NULL && taxSlider != NULL) {

            // Add Product to List
            int amount = spinBox->value();
            Product product = pushButton->getProduct();

            product.taxtype = TaxType(taxSlider->value());
            if (product.taxtype == TaxType::Standard)
                product.tax = settings.value("taxStandard", 19.0).toFloat();
            else if (product.taxtype == TaxType::Landwirtschaft)
                product.tax = settings.value("taxLandwirtschaft", 5.0).toFloat();
            else
                product.tax = 0.0;

            qDebug() << "ProductCount: "<< amount << product.title;
            bonmodel->insertProduct(amount, product);

            // Reset count
            spinBox->setValue(1);
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Check if doubleClicked outside of the QTableView cell area
    if (obj == this->ui->productTable->viewport()) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton)
                return false;

            QPoint point = this->ui->productTable->viewport()->mapFromGlobal(mouseEvent->globalPosition().toPoint());
            QModelIndex index = this->ui->productTable->indexAt(point);

            // Check if pos outside index
            if (!index.isValid()) {
                this->productmodel->insertRow(this->productmodel->rowCount());
            } else {
                this->ui->productTable->edit(index);
            }
            return true;
        } else {
            return false;
        }
    }
    else if (obj == this->ui->bonTable->viewport()) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton)
                return false;

            QPoint point = this->ui->bonTable->viewport()->mapFromGlobal(mouseEvent->globalPosition().toPoint());
            QModelIndex index = this->ui->bonTable->indexAt(point);

            // Check if pos outside index
            if (!index.isValid()) {
                this->bonmodel->insertRows(this->bonmodel->rowCount(QModelIndex()), 1);
            } else {
                this->ui->bonTable->edit(index);
            }
            return true;
        } else {
            return false;
        }
    }
    else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}

void MainWindow::customMenuRequested(QPoint pos)
{
    QModelIndex index = this->ui->productTable->indexAt(pos);

    QMenu *menu = new QMenu(this);
    QAction *newAction = new QAction("Neuen Eintrag erstellen", this);
    connect(newAction, &QAction::triggered,
        [this]() {
        qDebug("New Action!");
        this->productmodel->insertRow(0);
    });
    menu->addAction(newAction);

    if (index.row() != -1 && index.column() != -1) {
        QAction *deleteAction = new QAction("Eintrag löschen", this);
        connect(deleteAction, &QAction::triggered,
            [this, index]() {
            qDebug("Delete Action!");
            productmodel->removeRow(index.row());
            emit productmodel->dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            if (productmodel->submitAll()) {
                productmodel->database().commit();
            } else {
                productmodel->database().rollback();
                qDebug() << "Database Write Error" <<
                            "The database reported an error: " <<
                            productmodel->lastError().text();
            }

        });
        menu->addAction(deleteAction);
    }
    menu->popup(this->ui->productTable->viewport()->mapToGlobal(pos));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Change the table selector
    int index = (event->key()-0x30);
    if (index > 0 && index <= ui->tableComboBox->count()) {
        if (event->modifiers() == Qt::ControlModifier)
            ui->tabWidget->setCurrentIndex(index-1);
        else
            ui->tableComboBox->setCurrentIndex(index-1);
    }

    // Delete rows
    bool deleteFlag = event->modifiers() == Qt::ControlModifier;

    if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        qDebug() << "Deleting row(s)...";
        if (this->ui->bonTable->hasFocus()) {
            QModelIndex index = this->ui->bonTable->currentIndex();

            if (!deleteFlag) {
                QMessageBox msgBox;
                msgBox.setText("Soll die Zeile wirklich gelöscht werden?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                deleteFlag = (msgBox.exec() == QMessageBox::Yes);
            }

            if (deleteFlag)
                this->bonmodel->removeRow(index.row());
        }
        if (this->ui->productTable->hasFocus()) {
            QModelIndex index = this->ui->productTable->currentIndex();

            if (!deleteFlag) {
                QMessageBox msgBox;
                msgBox.setText("Soll die Zeile wirklich gelöscht werden?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                deleteFlag = (msgBox.exec() == QMessageBox::Yes);
            }

            if (deleteFlag) {
                this->productmodel->removeRow(index.row());
                this->productmodel->select();
            }
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateProductButton(int buttonId, Product *product, bool save)
{
    qDebug("Signal: updateProductButton triggered!");
    if (buttonId != -1) {
        AddProductButton *pushButton = getProductPushButton(buttonId);
        QLabel *priceLabel = getPriceLabel(buttonId);
        QSlider *taxSlider = getTaxSlider(buttonId);

        // Select product from list if selected
        if (product == nullptr) {
            QModelIndexList selection = this->ui->productTable->selectionModel()->selectedRows();
            if (selection.count() == 1) {
                product = new Product();
                product->title = this->productmodel->index(selection.at(0).row(), 1).data().toString();
                product->price = this->productmodel->index(selection.at(0).row(), 2).data().toFloat();

                pushButton->setProduct(product, false);
            } else {
                return;
            }
        }
        if (pushButton != NULL && product->title != "")
            pushButton->setLabelText(product->title);

        if (priceLabel != NULL)
            priceLabel->setText(product->getPriceFormatted());

        if (taxSlider != NULL) {
            switch (product->taxtype) {
            case TaxType::Standard:
                taxSlider->setValue(2);
            case TaxType::Landwirtschaft:
                taxSlider->setValue(1);
            case TaxType::Ohne:
                taxSlider->setValue(0);
            default:
                taxSlider->setValue(2);
            }
            //taxSlider->setValue(static_cast<int>(product->taxtype));
        }

        if (save && (pushButton != NULL || priceLabel != NULL)) {
            this->ui->tabWidget->saveSettings();
        } else {
            qDebug() << "Not all button updates saved!";
        }
    }
}

void MainWindow::updateSumLineEdit()
{
    QString sumText = QLocale::system().toString(bonmodel->getSum(), 'f', 2);
    ui->sumLineEdit->setText(sumText + " €");
}

void MainWindow::on_prevTableButton_clicked()
{
    int currentIndex = ui->tableComboBox->currentIndex();
    if (currentIndex > 0) {
        ui->tableComboBox->setCurrentIndex(currentIndex - 1);
    }
}

void MainWindow::on_nextTableButton_clicked()
{
    int currentIndex = ui->tableComboBox->currentIndex();
    if (currentIndex < ui->tableComboBox->count()-1) {
        ui->tableComboBox->setCurrentIndex(currentIndex + 1);
    }
}

void MainWindow::on_cashOutButton_clicked()
{
    // Printing
    QPrinter printer;

    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted) {

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.sss");

        QSqlQuery bonQuery;
        bool ok = bonQuery.exec("SELECT MAX(bonid) FROM Bons");
        ok &= bonQuery.first();
        if (!ok) {
            QMessageBox::warning(this, tr("sBon"),
                                 tr("The database reported an error: %1").arg(logmodel->lastError().text()));
            return;
        }
        const int bonid = bonQuery.value(0).toInt() + 1;

        // Get products from table
        QList<Product> products = this->bonmodel->getProducts();
        float sum = this->bonmodel->getSum();
        QHash<TaxType, float> taxes{};

        // Save bon to log
        QList<Product>::iterator i;
        QSqlQuery q;
        for (i = products.begin(); i != products.end(); ++i) {
            qDebug() << "title: " << (*i).title << "tax: " << (*i).tax;
            addBon(q, bonid, (*i).amount, (*i).title, (*i).price, (*i).tax, timestamp);

            if (taxes.contains((*i).taxtype))
                taxes[(*i).taxtype] += (*i).amount * (*i).price * (*i).tax / 100;
            else
                taxes[(*i).taxtype] = (*i).amount * (*i).price * (*i).tax / 100;
        }

        // Commit DB Changes
        logmodel->database().transaction();
        if (logmodel->submitAll()) {
            logmodel->database().commit();
        } else {
            logmodel->database().rollback();
            QMessageBox::warning(this, tr("sBon"),
                                 tr("The database reported an error: %1")
                                 .arg(logmodel->lastError().text()));
            return;
        }

        // print ...
        QPainter painter;
        if (! painter.begin(&printer)) { // failed to open file
           qWarning("failed to open file, is it writable?");

        }
        painter.setPen(QPen(Qt::black, 1.5, Qt::DashLine));

        QSettings s = QSettings("/tmp/sBon_Printing.ini", QSettings::NativeFormat);
        const int amountWidth = s.value("amountWidth", 20).toInt();
        const int titleWidth = s.value("titleWidth", 200).toInt();
        const int priceWidth = s.value("priceWidth", 70).toInt();
        const int leftBorder = s.value("leftBorder", 10).toInt();
        const int topBorder = s.value("topBorder", 15).toInt();
        const int bottomBorder = s.value("bottomBorder", 15).toInt();
        const int spacing = s.value("spacing", 10).toInt();
        const int lineSpacing = s.value("lineSpacing", 0).toInt();
        const int fontSize = s.value("fontSize", 12).toInt();
        const int headerFontSize = s.value("headerFontSize", 12).toInt();

        s.setValue("amountWidth", amountWidth);
        s.setValue("titleWidth", titleWidth);
        s.setValue("priceWidth", priceWidth);
        s.setValue("leftBorder", leftBorder);
        s.setValue("topBorder", topBorder);
        s.setValue("bottomBorder", bottomBorder);
        s.setValue("spacing", spacing);
        s.setValue("lineSpacing", lineSpacing);
        s.setValue("fontSize", fontSize);
        s.setValue("headerFontSize", headerFontSize);
        s.sync();

        int totalWidth = leftBorder + amountWidth + spacing + titleWidth + spacing + priceWidth;

        QFont font(QStringList{"Fake Receipt"}, fontSize);
        QFont headerFont(QStringList{"Arial"}, headerFontSize);

        QFontMetrics fm(font);
        int pixelsHigh = fm.height();

        int pixelsX, pixelsY;
        pixelsX = 0;
        pixelsY = 0;

        // Header
        painter.setFont(headerFont);
        QString textHeader("Obstweinschänke Vierbach\nTel 05652 - 992912\nE-Mail info@obstweinschaenke-vierbach.de");
        QRect recHeader = fm.boundingRect(QRect(0,0,totalWidth,400), Qt::AlignCenter, textHeader);
        painter.drawText(QRect(pixelsX, pixelsY, recHeader.width(), recHeader.height()), Qt::AlignCenter, textHeader);
        pixelsY += recHeader.height() + topBorder;

        painter.drawLine(0, pixelsY, totalWidth, pixelsY);
        pixelsY += topBorder;

        painter.setFont(font);
        for (int i=0; i<products.length(); ++i) {
            auto product = products[i];
            pixelsX = leftBorder;

            QRect recAmount = fm.boundingRect(QRect(0,0,amountWidth,100), Qt::AlignHCenter, QString::number(product.amount));
            painter.drawText(QRect(pixelsX, pixelsY, recAmount.width(), recAmount.height()), Qt::AlignHCenter, QString::number(product.amount));

            pixelsX += spacing + amountWidth;
            QRect recTitle = fm.boundingRect(QRect(0,0,titleWidth,100), Qt::TextWordWrap, product.title);
            painter.drawText(QRect(pixelsX, pixelsY, recTitle.width(), recTitle.height()), Qt::TextWordWrap, product.title);

            pixelsX += spacing + titleWidth;
            QRect recPrice = fm.boundingRect(QRect(0,0,priceWidth,100), Qt::AlignRight, product.getPriceFormatted());
            painter.drawText(QRect(pixelsX, pixelsY, recPrice.width(), recPrice.height()), Qt::AlignRight, product.getPriceFormatted());

            pixelsX += spacing + priceWidth;
            pixelsY += lineSpacing + qMax<qreal>(qMax<qreal>(recAmount.height(), recTitle.height()), recPrice.height());
        }
        pixelsY += bottomBorder;

        // Print Sum and Taxes
        painter.drawLine(0, pixelsY, totalWidth, pixelsY);
        pixelsY += bottomBorder;

        pixelsX = totalWidth/2;

        QString desc("Summe\n\nMWST%\n19%\n5%");
        QRect descRect = fm.boundingRect(QRect(0,0,100,100), Qt::TextWordWrap, desc);
        painter.drawText(QRect(pixelsX, pixelsY, descRect.width(), descRect.height()), desc);

        painter.drawLine(pixelsX-leftBorder, pixelsY+fm.lineSpacing()+3, totalWidth, pixelsY+fm.lineSpacing()+3);
        painter.drawLine(pixelsX-leftBorder, pixelsY+fm.lineSpacing()+6, totalWidth, pixelsY+fm.lineSpacing()+6);

        pixelsX = totalWidth - priceWidth;

        QString sumTax = QString("%1\n\n\n%2\n%3").arg(
            QLocale::system().toString(sum, 'f', 2) + "€",
            QLocale::system().toString(taxes[TaxType::Standard], 'f', 2) + "€",
            QLocale::system().toString(taxes[TaxType::Landwirtschaft], 'f', 2) + "€"
            );
        QRect sumTaxRect = fm.boundingRect(QRect(0,0,priceWidth,100), Qt::AlignRight, sumTax);
        painter.drawText(QRect(pixelsX, pixelsY, sumTaxRect.width(), sumTaxRect.height()), Qt::AlignRight, sumTax);
        painter.end();

        // Clear table
        bonmodel->clearTable();
    }
}

void MainWindow::printMonth(int month, int year)
{
    // Querying
    QSqlQueryModel model;
    model.setQuery(
        QString("SELECT * FROM Bons WHERE STRFTIME('%Y-%m', billed_at) IN ('%1-%2') ORDER BY datetime(billed_at) ASC").arg(
            QString::number(year), QString::number(month).rightJustified(2, '0'))
    );

    if (model.lastError().isValid())
        qDebug() << model.lastError();

    // Printing
    QPrinter printer;

    QPrintDialog printDialog(&printer);
    if (printDialog.exec() == QDialog::Accepted) {
        // print ...
        QPainter painter;
        if (! painter.begin(&printer)) { // failed to open file
           qWarning("failed to open file, is it writable?");
        }

        const double maxPageSize = printer.pageLayout().paintRect().height();
        const double topBorder = printer.pageLayout().paintRect().top();
        const double leftBorder = printer.pageLayout().paintRect().left();
        const int fontSize = 12;

        QFont font(QStringList{"Arial"}, fontSize);
        QFontMetrics fm(font);
        painter.setFont(font);

        int pixelsHigh = fm.height();

        int row = model.rowCount(QModelIndex());
        int x = leftBorder;
        int y = topBorder;

        int bonid = -1;
        double sum = 0.0;
        QHash<TaxType, float> taxes{};
        for (int i = 0; i < row ; ++i) {
            QSqlRecord record = model.record(i);

            if (bonid == -1 || record.value("bonid").toInt() == bonid) {
                int amount = record.value("amount").toInt();
                float price = record.value("price").toFloat();
                sum += amount * price;

                float tax = record.value("tax").toFloat();
                TaxType taxtype = getTaxType(tax);

                taxes[taxtype] += amount * price * tax / 100;

            } else {
                QString billed_at = record.value("billed_at").toString();
                QDateTime dt = QDateTime::fromString(billed_at, "yyyy-MM-dd hh:mm:ss.sss");

                painter.drawText(leftBorder+10, y, QString::number(bonid));
                painter.drawText(leftBorder+60, y, billed_at);
                painter.drawText(leftBorder+256, y, QString::number(sum));

                painter.drawText(leftBorder+300, y, QString::number(taxes[TaxType::Standard]));
                painter.drawText(leftBorder+350, y, QString::number(taxes[TaxType::Landwirtschaft]));

                y += pixelsHigh+5;

                sum = 0.0;
                taxes = {};
            }

            bonid = record.value("bonid").toInt();
        }


        if (! printer.newPage()) {
           qWarning("failed in flushing page to disk, disk full?");
        }
        painter.drawText(10, 10, "Test 2");

        painter.end();
    }
}

void MainWindow::on_tableComboBox_currentTextChanged(const QString& name) {
    bool ok = false;
    int index = 0;
    QRegularExpression re("(\\d+)");
    QRegularExpressionMatch match = re.match(name);
    if (match.hasMatch()) {
        auto i = match.captured(1).toInt(&ok);
        if (ok) index = i;
    }
    qDebug() <<  QString("Selecting different table: %1").arg(index);
    if (!this->bonmodel->setTable(index)) {
        // Reset to old table
    } else {
        settings.setValue("currentTableIndex", index);
    }
    updateSumLineEdit();
}

void MainWindow::on_sortCheckBox_toggled(bool checked)
{
    this->ui->bonTable->setSortingEnabled(checked);
    if (!checked) {
        bonsearchmodel->setSortRole(Qt::InitialSortOrderRole);
        bonsearchmodel->invalidate();
    }
}

AddProductButton *MainWindow::getProductPushButton(int buttonId) {
    QString buttonObjectName = "addProductButton_"+QString::number(buttonId);
    AddProductButton *button = this->ui->tabWidget->findChild<AddProductButton *>(buttonObjectName);
    return button;
}

QSpinBox *MainWindow::getAmountSpinBox(int buttonId) {
    QString spinBoxObjectName = "amountSpinBox_"+QString::number(buttonId);
    QSpinBox *spinBox = this->ui->tabWidget->findChild<QSpinBox *>(spinBoxObjectName);
    return spinBox;
}

QSlider *MainWindow::getTaxSlider(int buttonId) {
    QString sliderObjectName = "taxSlider_"+QString::number(buttonId);
    QSlider *slider = this->ui->tabWidget->findChild<QSlider *>(sliderObjectName);
    return slider;
}

QLabel *MainWindow::getPriceLabel(int buttonId) {
    QString priceLabelName = "priceLabel_"+QString::number(buttonId);
    QLabel *priceLabel = ui->tabWidget->findChild<QLabel *>(priceLabelName);
    return priceLabel;
}


