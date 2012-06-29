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
#include <QGraphicsPixmapItem>
#include <QIcon>
#include <QTextEdit>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
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
#include "PatchProjection/EigenHelpers/EigenHelpers.h"
#include "PatchProjection/PatchProjection.h"
#include "vtkPointSelectionStyle/PointSelectionStyle2D.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

void InteractivePatchProjectionWidget::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();

  help->setReadOnly(true);
  help->append("<h1>Interactive Patch Comparison</h1>\
  Click on the image. The patch centered around the clicked pixel is projected<br/>\
  using the basis computed from the image patches.<p/>");
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

  this->OriginalPatchScene = new QGraphicsScene;
  this->ProjectedPatchScene = new QGraphicsScene;

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");

  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);
  actionOpenMask->setEnabled(false);

  // Setup the image display objects
  this->ImageLayer.ImageSlice->VisibilityOff(); // There are errors if this is visible and therefore displayed before it has data ("This data object does not contain the requested extent.")

  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();

  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->InteractorStyle = vtkSmartPointer<PointSelectionStyle2D>::New();
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  //this->InteractorStyle->SetDefaultRenderer(this->Renderer);
  this->InteractorStyle->SetInteractor(this->qvtkWidget->GetRenderWindow()->GetInteractor());

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);

  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);

  this->Image = ImageType::New();

  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Connections->Connect(this->InteractorStyle,
                             this->InteractorStyle->ClickedPointEvent,
                             this,
                             SLOT(slot_clicked(vtkObject*, unsigned long, void*, void*)));
}

void InteractivePatchProjectionWidget::ComputeProjectedPatch()
{
  // Compute the projected patch
  Eigen::VectorXf vectorized = PatchProjection<Eigen::MatrixXf, Eigen::VectorXf>::VectorizePatch(this->Image.GetPointer(),
                                                               this->SelectedRegion);
  vectorized -= this->MeanVector; // Subtract the mean

  //EigenHelpers::OutputHorizontal("vectorized", vectorized);

  unsigned int numberOfDimensionsToProjectTo = this->sldDimensions->value();
  //std::cout << "numberOfDimensionsToProjectTo: " << numberOfDimensionsToProjectTo << std::endl;

  Eigen::MatrixXf truncatedProjectionMatrix =
         EigenHelpers::TruncateColumns(this->ProjectionMatrix, numberOfDimensionsToProjectTo);
  // This is a change of basis, hence the transpose
  Eigen::VectorXf projectedVector = truncatedProjectionMatrix.transpose() * vectorized;

//  EigenHelpers::OutputHorizontal("projectedVector", projectedVector);

  // This is "undoing" the transformation
  Eigen::VectorXf unprojectedVector = truncatedProjectionMatrix * projectedVector;

  // Apply the inverse normalization transform

  unprojectedVector += this->MeanVector; // Add the mean

  //EigenHelpers::OutputHorizontal("unprojectedVector", unprojectedVector);

  ImageType::Pointer projectedPatchImage = ImageType::New();
  PatchProjection<Eigen::MatrixXf, Eigen::VectorXf>::UnvectorizePatch(unprojectedVector, projectedPatchImage.GetPointer(),
                                    this->Image->GetNumberOfComponentsPerPixel());

  // Display the projected patch
  QImage projectedPatchQImage = ITKQtHelpers::GetQImageColor(projectedPatchImage.GetPointer(),
                                                             projectedPatchImage->GetLargestPossibleRegion());

}

void InteractivePatchProjectionWidget::DisplayPatches()
{
  // This function assumse that this->SelectedRegion is set properly

  // Display the original patch
  this->OriginalPatchQImage = ITKQtHelpers::GetQImageColor(this->Image.GetPointer(), this->SelectedRegion);

  QGraphicsPixmapItem* originalPatchItem =
        this->OriginalPatchScene->addPixmap(QPixmap::fromImage(this->OriginalPatchQImage));
  this->gfxOriginalPatch->setScene(this->OriginalPatchScene);
  this->gfxOriginalPatch->fitInView(originalPatchItem);

  // Display the projected patch
  QGraphicsPixmapItem* projectedPatchItem =
           this->ProjectedPatchScene->addPixmap(QPixmap::fromImage(this->ProjectedPatchQImage));
  this->gfxProjectedPatch->setScene(this->ProjectedPatchScene);
  this->gfxProjectedPatch->fitInView(projectedPatchItem);
}

void InteractivePatchProjectionWidget::on_sldDimensions_sliderReleased()
{
  // If the image has not been clicked yet, we can't do the computation.
  if(this->SelectedRegion.GetSize()[0] == 0)
  {
    return;
  }

  DisplayPatches();
}

void InteractivePatchProjectionWidget::on_sldDimensions_valueChanged(int value)
{
  // We don't want to handle every value as the slider is being moved with the mouse, only where it ends up.
  if(this->sldDimensions->horizontalSlider->isSliderDown())
  {
    return;
  }
  // std::cout << "Slider moved." << std::endl;

  // If the image has not been clicked yet, we can't do the computation.
  if(this->SelectedRegion.GetSize()[0] == 0)
  {
    return;
  }

  DisplayPatches();
}

void InteractivePatchProjectionWidget::slot_clicked(vtkObject* caller, unsigned long eventId,
                                                    void* client_data, void* call_data)
{
  // We expect this slot to be called with 'call_data' containing the location of the picked
  // point in image coordinates.

  // Get the picked region
  double* point = reinterpret_cast<double*>(call_data);
  //std::cout << "Picked: " << point[0] << " " << point[1] << " " << point[2] << std::endl;

  itk::Index<2> index = {{static_cast<int>(point[0]), static_cast<int>(point[1])}};

  this->SelectedRegion = ITKHelpers::GetRegionInRadiusAroundPixel(index, GetPatchRadius());

  // We can't process a patch that is not fully inside the image.
  if(!this->Image->GetLargestPossibleRegion().IsInside(this->SelectedRegion))
  {
    itk::Index<2> zeroIndex = {{0,0}};
    itk::Size<2> zeroSize = {{0,0}};
    this->SelectedRegion.SetIndex(zeroIndex);
    this->SelectedRegion.SetSize(zeroSize);
    return;
  }

  ComputeProjectedPatch();
  DisplayPatches();
}

InteractivePatchProjectionWidget::InteractivePatchProjectionWidget(QWidget* parent) : QMainWindow(parent)
{
  SharedConstructor();
}

void InteractivePatchProjectionWidget::on_actionQuit_activated()
{
  exit(0);
}

void InteractivePatchProjectionWidget::resizeEvent(QResizeEvent* event)
{
  DisplayPatches();
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

  std::cout << "Working directory set to: " << workingDirectory << std::endl;
  QDir::setCurrent(QString(workingDirectory.c_str()));

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileInfo.fileName().toStdString());
  reader->Update();

  ITKHelpers::DeepCopy(reader->GetOutput(), this->Image.GetPointer());

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image.GetPointer(), this->ImageLayer.ImageData);

  this->statusBar()->showMessage("Opened image.");

  this->ImageLayer.ImageSlice->VisibilityOn();

  std::cout << "Computing projection matrix with radius = " << GetPatchRadius() << std::endl;
  // NOTE: this will crash if the patch size is too big (too big for RAM in a machine with 4GB).
  // Known to work with radius=7, known to not work with radius=15
  this->ProjectionMatrix = PatchProjection<Eigen::MatrixXf, Eigen::VectorXf>::ComputeProjectionMatrix_CovarianceEigen(this->Image.GetPointer(),
                                                                                    GetPatchRadius(),
                                                                                    this->MeanVector);

  //std::cout << "Mean: " << this->MeanVector << std::endl;

  std::cout << "Projection matrix is " << this->ProjectionMatrix.rows()
            << " x " << this->ProjectionMatrix.cols() << std::endl;

  this->sldDimensions->setMinimum(1);
  this->sldDimensions->setMaximum(this->ProjectionMatrix.rows());
}

void InteractivePatchProjectionWidget::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName =
           QFileDialog::getOpenFileName(this,
                                        "Open File", ".",
                                        "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

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
