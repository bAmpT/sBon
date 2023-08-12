#ifndef PRODUCTTABWIDGET_H
#define PRODUCTTABWIDGET_H

#include <QTabWidget>
#include <QtWidgets>
#include "addproductbutton.h"

class ProductTabWidget : public QTabWidget
{
public:
    ProductTabWidget(QWidget *parent = nullptr);
    void addNewTab(const QString& name, int numButtons = 30);

    void saveSettings();
    void loadSettings();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QList<QList<QWidget*>> buttonAreasList = {};
    uint totalButtonCount = 0;

    void showContextMenu(const QPoint &point);
    QMainWindow* getMainWindow();

    //QSettings settings;// = QSettings("sBon_ProductTabWidget");
};

#endif // PRODUCTTABWIDGET_H
