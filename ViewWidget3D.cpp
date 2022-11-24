#include "ViewWidget3D.h"
#include "ui_ViewWidget3D.h"

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSTLReader.h>

#include <QLabel>

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

// TODO current this function
void ViewWidget3D::loadDEM(std::string fileName)
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
