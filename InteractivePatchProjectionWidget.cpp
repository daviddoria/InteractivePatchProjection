/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "InteractivePatchProjectionWidget.h"

// Eigen
#include <Eigen/Dense>

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>

// Submodules
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "EigenHelpers/EigenHelpers.h"

void InteractivePatchProjectionWidget::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();

  help->setReadOnly(true);
  help->append("<h1>Interactive Patch Comparison</h1>\
  Position the two patches. <br/>\
  Their difference will be displayed.<br/>\
  Additionally you can display the top patches of the target patch.<p/>");
  help->show();
}

// Constructors
InteractivePatchProjectionWidget::InteractivePatchProjectionWidget(const std::string& imageFileName,
                                                                   QWidget* parent)
: QMainWindow(parent)
{
  SharedConstructor();
  OpenImage(imageFileName);
}

void InteractivePatchProjectionWidget::SharedConstructor()
{
  this->setupUi(this);

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");

  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);
  actionOpenMask->setEnabled(false);
/*
  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);*/

  this->InteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();

  // Setup the image display objects
  this->ImageLayer.ImageSlice->PickableOff();
  this->ImageLayer.ImageSlice->VisibilityOff(); // There are errors if this is visible and therefore displayed before it has data ("This data object does not contain the requested extent.")

  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);

  this->Image = ImageType::New();

}

InteractivePatchProjectionWidget::InteractivePatchProjectionWidget(QWidget* parent) : QMainWindow(parent)
{
  SharedConstructor();
};

void InteractivePatchProjectionWidget::on_actionQuit_activated()
{
  exit(0);
}

void InteractivePatchProjectionWidget::showEvent(QShowEvent* event)
{
  GetPatchSize();

  this->Renderer->ResetCamera();

  Refresh();
}

void InteractivePatchProjectionWidget::OpenImage(const std::string& fileName)
{
  // Set the working directory
  QFileInfo fileInfo(fileName.c_str());
  std::string workingDirectory = fileInfo.absoluteDir().absolutePath().toStdString() + "/";

  //std::cout << "Working directory set to: " << workingDirectory << std::endl;
  QDir::setCurrent(QString(workingDirectory.c_str()));

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  ITKHelpers::DeepCopy(reader->GetOutput(), this->Image.GetPointer());

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image.GetPointer(), this->ImageLayer.ImageData);

  this->statusBar()->showMessage("Opened image.");

  this->ImageLayer.ImageSlice->VisibilityOn();
}

void InteractivePatchProjectionWidget::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

  //std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  OpenImage(fileName.toStdString());
}

void InteractivePatchProjectionWidget::on_txtPatchRadius_returnPressed()
{

}

unsigned int InteractivePatchProjectionWidget::GetPatchRadius()
{
  return this->txtPatchRadius->text().toUInt();
}

void InteractivePatchProjectionWidget::GetPatchSize()
{
  // The edge length of the patch is the (radius*2) + 1
  this->PatchSize[0] = GetPatchRadius() * 2 + 1;
  this->PatchSize[1] = GetPatchRadius() * 2 + 1;
}

void InteractivePatchProjectionWidget::RefreshSlot()
{
  Refresh();
}

void InteractivePatchProjectionWidget::Refresh()
{
  this->qvtkWidget->GetRenderWindow()->Render();
}
