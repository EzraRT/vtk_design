#include "ViewWidget3D.h"
#include "ui_ViewWidget3D.h"

#include <algorithm>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCylinderSource.h>
#include <vtkFloatArray.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSTLReader.h>

#include <gdal_priv.h>

#include <QLabel>
#include <QMessageBox>

ViewWidget3D::ViewWidget3D(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::ViewWidget3D)
{
    ui->setupUi(this);
    vtkWidget = new QVTKOpenGLNativeWidget();
    this->setCentralWidget(vtkWidget);
    vtkNew<vtkNamedColors> colors;

    // Set the background color.
    std::array<unsigned char, 4> bkg { { 26, 51, 102, 255 } };
    colors->SetColor("BkgColor", bkg.data());

    // This creates a polygonal cylinder model with eight circumferential facets
    // (i.e, in practice an octagonal prism).
    vtkNew<vtkCylinderSource> cylinder;
    cylinder->SetResolution(8);

    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes are
    // defined.
    vtkNew<vtkPolyDataMapper> cylinderMapper;
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());

    // The actor is a grouping mechanism: besides the geometry (mapper), it
    // also has a property, transformation matrix, and/or texture map.
    // Here we set its color and rotate it around the X and Y axes.
    vtkNew<vtkActor> cylinderActor;
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(
        colors->GetColor4d("Green").GetData());
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);

    // The renderer generates the image
    // which is then displayed on the render window.
    // It can be thought of as a scene to which the actor is added
    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(cylinderActor);
    renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
    // Zoom in a little by accessing the camera and invoking its "Zoom" method.
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Zoom(1.5);

    vtkWidget->renderWindow()->AddRenderer(renderer);
}

#define dbg(x) QMessageBox::warning(this, tr("Warning"), QString::number(x))
#define dbgstr(x) QMessageBox::warning(this, tr("Warning"), QString::fromStdString(x))

void ViewWidget3D::loadDEM(std::string fileName)
{
    GDALAllRegister();
    GDALDataset* dataset = (GDALDataset*)GDALOpen(fileName.c_str(), GA_ReadOnly);

    if (dataset == nullptr) {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to open file!"));
        return;
    }

    double padTransform[6];
    dataset->GetGeoTransform(padTransform);

    auto band = dataset->GetRasterBand(1);
    auto width = band->GetXSize();
    auto height = band->GetYSize();
    auto array = band->AsMDArray();

    vtkNew<vtkPolyData> polyData;
    polyData->SetLines(vtkNew<vtkCellArray>());
    polyData->SetPolys(vtkNew<vtkCellArray>());

    auto arrbuf = new double[width * height];

    band->RasterIO(GF_Read, 0, 0, width, height, arrbuf, width, height, GDT_Float64, 0, 0);

    auto baseData = array->at(0, 0);

    double maxHeight = baseData->GetNoDataValueAsDouble(), minHeight = baseData->GetNoDataValueAsDouble();

    vtkNew<vtkPoints> points;

    for (size_t yPixel = 0; yPixel < height; yPixel++) {
        for (size_t xPixel = 0; xPixel < width; xPixel++) {
            auto data = arrbuf[yPixel * width + xPixel];
            data = std::max(data, -200.0);
            auto xGeo = padTransform[0] + xPixel * padTransform[1] + yPixel * padTransform[2];
            auto yGeo = padTransform[3] + xPixel * padTransform[4] + yPixel * padTransform[5];
            points->InsertNextPoint(xGeo, yGeo, data / 10000);
            minHeight = std::min(minHeight, data);
            maxHeight = std::max(maxHeight, data);
        }
    }

    auto heightRange = maxHeight - minHeight;

    polyData->SetPoints(points);

    vtkNew<vtkFloatArray> colors;
    polyData->GetCellData()->SetScalars(colors);

    vtkNew<vtkIdList> ids;

    // set polygon info
    for (size_t yPixel = 0; yPixel < height - 1; yPixel++) {
        for (size_t xPixel = 0; xPixel < width - 1; xPixel++) {
            auto data = arrbuf[yPixel * width + xPixel];
            data = std::max(data, -200.0);
            auto colorScalar = 8 * (data - minHeight) / heightRange;
            auto indexTL = yPixel * width + xPixel;
            auto indexTR = yPixel * width + xPixel + 1;
            auto indexBL = indexTL + width;
            auto indexBR = indexTR + width;
            ids->Reset();
            ids->InsertNextId(indexTL);
            ids->InsertNextId(indexBL);
            ids->InsertNextId(indexBR);
            auto cell = polyData->InsertNextCell(VTK_POLYGON, ids);
            colors->InsertTuple1(cell, colorScalar);
            ids->Reset();
            ids->InsertNextId(indexTL);
            ids->InsertNextId(indexTR);
            ids->InsertNextId(indexBR);
            auto nextcell = polyData->InsertNextCell(VTK_POLYGON, ids);
            colors->InsertTuple1(nextcell, colorScalar);
        }
    }

    delete[] arrbuf;

    vtkNew<vtkLookupTable> lut;
    lut->SetNumberOfTableValues(9);
    lut->SetTableRange(0, 8);
    lut->SetTableValue(0, 0.517, 0.710, 0.694, 1.0);
    lut->SetTableValue(1, 0.765, 0.808, 0.572, 1.0);
    lut->SetTableValue(2, 0.086, 0.521, 0.149, 1.0);
    lut->SetTableValue(3, 0.580, 0.580, 0.141, 1.0);
    lut->SetTableValue(4, 0.721, 0.266, 0.027, 1.0);
    lut->SetTableValue(5, 0.396, 0.098, 0.003, 1.0);
    lut->SetTableValue(6, 0.474, 0.278, 0.149, 1.0);
    lut->SetTableValue(7, 0.694, 0.686, 0.698, 1.0);
    lut->SetTableValue(8, 1.0, 1.0, 1.0, 1.0);
    lut->Build();

    vtkNew<vtkPolyDataMapper> polyDataMapper;
    polyDataMapper->SetInputData(polyData);
    polyDataMapper->SetScalarModeToUseCellData();
    polyDataMapper->UseLookupTableScalarRangeOn();
    polyDataMapper->SetLookupTable(lut);

    vtkNew<vtkActor> actor;
    actor->SetMapper(polyDataMapper);

    vtkWidget->renderWindow()->RemoveRenderer(vtkWidget->renderWindow()->GetRenderers()->GetFirstRenderer());

    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Zoom(1.5);
    vtkWidget->renderWindow()->AddRenderer(renderer);
    vtkWidget->renderWindow()->Render();
}

void ViewWidget3D::loadSTL(std::string fileName)
{
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);

    vtkWidget->renderWindow()->RemoveRenderer(vtkWidget->renderWindow()->GetRenderers()->GetFirstRenderer());
    vtkNew<vtkRenderer> renderer;
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.2, 0.4);
    renderer->ResetCamera();
    renderer->GetActiveCamera()->Zoom(1.5);
    vtkWidget->renderWindow()->AddRenderer(renderer);
    vtkWidget->renderWindow()->Render();
}

ViewWidget3D::~ViewWidget3D()
{
    delete vtkWidget;
    delete ui;
}

QVTKOpenGLNativeWidget* ViewWidget3D::getWidget()
{
    return vtkWidget;
}
