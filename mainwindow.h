#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "producttablemodel.h"
#include "bontablemodel.h"
#include "addproductbutton.h"

#include <QMainWindow>
#include <QtWidgets>
#include <QtSql>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

template <typename T, typename D = T>
class Singleton
{
    friend D;
    static_assert(std::is_base_of_v<T, D>, "T should be a base type for D");

public:
    static T& instance();

private:
    Singleton() = default;
    ~Singleton() = default;
    Singleton( const Singleton& ) = delete;
    Singleton& operator=( const Singleton& ) = delete;
};

template <typename T, typename D>
T& Singleton<T, D>::instance()
{
    static D inst;
    return inst;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addProduct();
    void updateProductButton(int buttonId, Product *product, bool save = true);
    void updateSumLineEdit();
    void customMenuRequested(QPoint pos);
    void printMonth(int month, int year);

    void on_cashOutButton_clicked();
    void on_prevTableButton_clicked();
    void on_nextTableButton_clicked();
    void on_tableComboBox_currentTextChanged(const QString& name);
    void on_sortCheckBox_toggled(bool checked);
    // Autoconnect:
    // void on_<object name>_<signal name>(<signal parameters>);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    ProductTableModel *productmodel;
    BonTableModel *bonmodel;
    QSortFilterProxyModel *bonsearchmodel;
    QSqlTableModel *logmodel;
    QComboBox *tableComboBox;
    QSettings settings;

    AddProductButton *getProductPushButton(int buttonId);
    QLabel *getPriceLabel(int buttonId);
    QSpinBox *getAmountSpinBox(int buttonId);
    QSlider *getTaxSlider(int buttonId);

    QHash<QString, QVariant> productIdTable;

    void showError(const QSqlError &err);
    void loadSettings();
    void writeSettings();
};

#endif // MAINWINDOW_H
