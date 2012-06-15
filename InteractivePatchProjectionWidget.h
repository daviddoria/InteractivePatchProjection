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

#ifndef InteractivePatchProjectionWidget_H
#define InteractivePatchProjectionWidget_H

#include "ui_InteractivePatchProjectionWidget.h"

// Eigen
#include <Eigen/Dense>

// VTK
#include <vtkSmartPointer.h>

class vtkEventQtSlotConnect;
class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;

// Custom
class PointSelectionStyle2D;

// ITK
#include "itkImage.h"
#include "itkVectorImage.h"

// Qt
#include <QMainWindow>
class QGraphicsScene;

// Submodules
#include "ITKVTKCamera/ITKVTKCamera.h"
#include "Layer/Layer.h"

class InteractivePatchProjectionWidget : public QMainWindow, private Ui::InteractivePatchProjectionWidget
{
Q_OBJECT
public:

  typedef itk::VectorImage<float, 2> ImageType;

  // Constructor/Destructor
  InteractivePatchProjectionWidget(QWidget* parent = 0);
  InteractivePatchProjectionWidget(const std::string& imageFileName, QWidget* parent = 0);
  void SharedConstructor();

  void Refresh();

public slots:

  void on_actionOpenImage_activated();

  void on_actionHelp_activated();

  void on_actionQuit_activated();

  void on_txtPatchRadius_returnPressed();

  void RefreshSlot();

  /** This slot is called when the image is clicked. */
  void slot_clicked(vtkObject* caller, unsigned long eventId, void* client_data, void* call_data);

  /** This slot is called when the slider is moved. */
  void on_sldDimensions_sliderReleased();

private:

  /** Display the patches. */
  void DisplayPatches();

  /** Get the radius of the patches being used. */
  unsigned int GetPatchRadius();

  /** This function is called when the widget is displayed on the screen. */
  void showEvent(QShowEvent* event);

  /** Open an image. */
  void OpenImage(const std::string& filename);

  /** Get the size of the patches being used from the GUI. */
  void GetPatchSize();

  /** This style invokes an event when the image is clicked that contains the point that was clicked in the call_data. */
  vtkSmartPointer<PointSelectionStyle2D> InteractorStyle;

  /** Allows us to change the camera orientation to display the image appropriately */
  ITKVTKCamera itkvtkCamera;

  /** The renderer */
  vtkSmartPointer<vtkRenderer> Renderer;

  /** Image display */
  Layer ImageLayer;

  /** The data that the user loads */
  ImageType::Pointer Image;

  /** The size of the patches to use */
  itk::Size<2> PatchSize;

  /** The projection matrix to project patches to a lower dimensional space */
  Eigen::MatrixXf ProjectionMatrix;

  /** The mean of each component of the vectorized image patches */
  Eigen::VectorXf MeanVector;

  /** The standard deviation of each component of the vectorized image patches */
  Eigen::VectorXf StandardDeviationVector;

  /** An object to allow us to connect Qt events to VTK callbacks */
  vtkSmartPointer<vtkEventQtSlotConnect> Connections;

  /** The scene for the original patch */
  QGraphicsScene* OriginalPatchScene;

  /** The scene for the projected patch */
  QGraphicsScene* ProjectedPatchScene;

  /** The region that has been selected */
  itk::ImageRegion<2> SelectedRegion;
};

#endif // InteractivePatchProjectionWidget_H
