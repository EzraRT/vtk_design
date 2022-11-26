#include "ModelViewer.h"
#include "ui_ModelViewer.h"

#include "ViewWidget3D.h"

#include <QFileDialog>
#include <QLayout>
#include <QMessageBox>
#include <QVBoxLayout>

ModelViewer::ModelViewer(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ModelViewer)
{
    ui->setupUi(this);

    viewWidget3D = new ViewWidget3D(this);

    QLayout* layout = new QVBoxLayout();
    layout->addWidget(viewWidget3D);

    ui->widget->setLayout(layout);
}

ModelViewer::~ModelViewer()
{
    delete viewWidget3D;
    delete ui;
}

void ModelViewer::on_pushButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters({ "DEM FIles (*.tif)", "STL Files (*.stl)" });
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
        fileNames = dialog.selectedFiles();
    else {
        return;
    }

    QString fileName = fileNames[0];

    if (fileName.endsWith(".stl")) {
        viewWidget3D->loadSTL(fileName.toStdString());
    } else if (fileName.endsWith(".tif")) {
        viewWidget3D->loadDEM(fileName.toStdString());
    } else {
        // unsupported file type
        // show warning dialog
        QMessageBox::warning(this, tr("Warning"), tr("Unsupported file type!"));
        return;
    }
}
