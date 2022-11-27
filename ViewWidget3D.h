#ifndef VIEWWIDGET3D_H
#define VIEWWIDGET3D_H

#include <QMainWindow>
#include <QWidget>

#include <QVTKOpenGLNativeWidget.h>

#include <array>

namespace Ui {
class ViewWidget3D;
}

class ViewWidget3D : public QMainWindow {
    Q_OBJECT

public:
    ViewWidget3D(QWidget* parent = nullptr);
    ~ViewWidget3D();

    QVTKOpenGLNativeWidget* getVTKWidget();
    void loadSTL(std::string fileName);
    void loadDEM(std::string fileName);

private:
    QVTKOpenGLNativeWidget* vtkWidget;
    Ui::ViewWidget3D* ui;
};

#endif // VIEWWIDGET3D_H
