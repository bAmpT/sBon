#include "producttabwidget.h"
#include "flowlayout.h"
#include "addproductbutton.h"
#include "mainwindow.h"

ProductTabWidget::ProductTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    // Callback for moving tabs
    QObject::connect(this->tabBar(), &QTabBar::tabMoved, [this]() {
        qDebug("Tab moved!");
        this->saveSettings();
    });
}

void ProductTabWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // handle right mouse button here
        QPoint point = this->tabBar()->mapFromGlobal(event->globalPosition().toPoint());

        int tabIndex = this->tabBar()->tabAt(point);
        if (tabIndex != -1) {
            QMenu menu(this);

            menu.addAction(tr("Neue Kategorie"), this, [this]() {
                bool ok;
                QString text = QInputDialog::getText(this, tr("Neuen Tab erstellen"),
                                                     tr("Titel der Kategorie:"), QLineEdit::Normal,
                                                     "", &ok);
                if (ok && !text.isEmpty()) {
                    this->addNewTab(text);
                    this->saveSettings();
                }
            });
            if (this->currentIndex() == tabIndex) {
                menu.addAction(tr("Umbenennen der Kategorie"), this, [this,tabIndex]() {
                    bool ok;
                    QString text = QInputDialog::getText(this, tr("Tab umbenennen"),
                                                         tr("Titel der Kategorie:"), QLineEdit::Normal,
                                                         this->tabText(tabIndex), &ok);
                    if (ok && !text.isEmpty()) {
                        this->setTabText(tabIndex, text);
                        this->saveSettings();
                    }
                });
                menu.addAction(tr("Löschen der Kategorie"), this, [this,tabIndex]() {
                    QMessageBox msgBox;
                    msgBox.setText("Soll die Kategorie gelöscht werden?");
                        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
                    msgBox.setDefaultButton(QMessageBox::Cancel);

                    if (msgBox.exec() == QMessageBox::Yes) {
                        this->removeTab(this->currentIndex());
                        this->saveSettings();
                    }
                });
            }

            menu.exec(this->tabBar()->mapToGlobal(point));
        }
    } else {
        // pass on other buttons to base class
        QTabWidget::mousePressEvent(event);
    }
}

void ProductTabWidget::loadSettings()
{
    QSettings settings("/tmp/sBon.ini", QSettings::NativeFormat);

    if (settings.contains("ProductTabWidget/tabTitles"))
    {
        QStringList tabsTitles = settings.value("ProductTabWidget/tabTitles").toStringList();
        qDebug() << "Loading Tabs ("<<tabsTitles.count()<<")";

        int total = 0;
        for (int i=0; i<tabsTitles.count(); i++) {
            QVariant buttonProductsVariant = settings.value(QString("ProductTabWidget/buttonProducts/%1").arg(i));
            QList<Product> buttonProducts = buttonProductsVariant.value<QList<Product>>();

            QString title = tabsTitles.at(i);
            int numButtons = buttonProducts.count();

            addNewTab(title, numButtons);

            QList<QWidget*> buttonAreas = buttonAreasList[i];
            for (int j=0; j<buttonAreas.count(); j++) {
                Product *p = new Product(buttonProducts.at(j));
                buttonAreas.at(j)->findChild<AddProductButton*>()->setProduct(p);
            }
        }
        if (tabsTitles.count() == 0) {
            addNewTab("Kategorie 1");
            saveSettings();
        }
    } else {
        addNewTab("Kategorie 1");
        saveSettings();
    }
}
void ProductTabWidget::saveSettings()
{
    QSettings settings("/tmp/sBon.ini", QSettings::NativeFormat);

    QStringList tabsTitles;
    int total = 0;
    for (int i=0; i<this->count(); i++) {
        tabsTitles.append(this->tabText(i));

        QStringList buttonTitles;
        QStringList buttonPrices;
        QList<Product> buttonProducts;

        QList<QWidget*> buttonAreas = buttonAreasList[i];//this->widget(i)->findChildren<QWidget*>(QRegularExpression("buttonArea_\\d+_"));
        foreach(QWidget* buttonArea, buttonAreas) {
            buttonProducts.append(buttonArea->findChild<AddProductButton*>()->getProduct());
        }
        settings.setValue(QString("ProductTabWidget/buttonProducts/%1").arg(i), QVariant::fromValue(buttonProducts));
    }
    settings.setValue("ProductTabWidget/tabTitles", tabsTitles);
    settings.sync();
}

QMainWindow* ProductTabWidget::getMainWindow()
{
    foreach(QWidget *widget, qApp->topLevelWidgets())
        if (QMainWindow *mainWindow = dynamic_cast<QMainWindow*>(widget))
            return mainWindow;
    return NULL;
}

void ProductTabWidget::addNewTab(const QString& name, int numButtons)
{
    QWidget *tabWidget = new QWidget();
    this->addTab(tabWidget, name);

    QVBoxLayout *vlayout = new QVBoxLayout;
    tabWidget->setLayout(vlayout);

    QScrollArea* scrollArea = new QScrollArea();
    vlayout->addWidget(scrollArea);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QWidget *widget = new QWidget();
    scrollArea->setWidget(widget);

    FlowLayout *flowLayout = new FlowLayout;
    widget->setLayout(flowLayout);

    // Find MainWindow handle
    QMainWindow *mainWindow = getMainWindow();

    QList<QWidget*> buttonAreas;
    int totalCount = numButtons+totalButtonCount;
    for (int i=totalButtonCount; i < totalCount; i++) {
        QWidget* buttonArea = new QWidget();
        buttonArea->setObjectName(QString("buttonArea_%1").arg(QString::number(i)));
        buttonArea->setFixedSize(QSize(271,81));

        AddProductButton *pushButton = new AddProductButton(buttonArea);
        pushButton->setObjectName(QString("addProductButton_%1").arg(i));
        pushButton->setGeometry(60,0,211,71);
        pushButton->setText("<Leer>");
        pushButton->setAcceptDrops(true);

        // Connect to MainWindow
        QObject::connect(pushButton, SIGNAL(clicked()), mainWindow, SLOT(addProduct()));
        QObject::connect(pushButton, SIGNAL(updatedProductButton(int, Product*)), mainWindow, SLOT(updateProductButton(int, Product*)));

        QLabel *label = new QLabel(buttonArea);
        label->setObjectName(QString("priceLabel_%1").arg(i));
        label->setGeometry(0,0,51,25);
        label->setAlignment(Qt::AlignHCenter);
        label->setText("0,00€");

        QSpinBox *spinBox = new QSpinBox(buttonArea);
        spinBox->setObjectName(QString("amountSpinBox_%1").arg(i));
        spinBox->setGeometry(0,25,51,24);
        spinBox->setAlignment(Qt::AlignHCenter);
        spinBox->setMinimum(1);

        QSlider *slider = new QSlider(buttonArea);
        slider->setObjectName(QString("taxSlider_%1").arg(i));
        slider->setGeometry(0,50,51,25);
        slider->setOrientation(Qt::Horizontal);
        slider->setTickPosition(QSlider::TicksAbove);
        slider->setPageStep(1);
        slider->setMinimum(0);
        slider->setMaximum(2);
        slider->setValue(2);

        // Connect to change value
        QObject::connect(slider, &QSlider::valueChanged, [this](int value) {
            qDebug("Tax slider value changed!");
            this->saveSettings();
        });

        flowLayout->addWidget(buttonArea);
        buttonAreas.append(buttonArea);
        totalButtonCount += 1;
    }
    buttonAreasList.append(buttonAreas);
}
