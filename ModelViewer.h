#ifndef MODELVIEWER_H
#define MODELVIEWER_H

#include "ViewWidget3D.h"

#include <QMainWindow>

namespace Ui {
class ModelViewer;
}

class ModelViewer : public QMainWindow {
    Q_OBJECT

public:
    explicit ModelViewer(QWidget* parent = nullptr);
    ~ModelViewer();

private slots:
    void on_pushButton_clicked();

private:
    ViewWidget3D* viewWidget3D;
    Ui::ModelViewer* ui;
};

#endif // #ifndef MODELVIEWER_H
