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

// VTK
#include <vtkSmartPointer.h>

class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;
class vtkInteractorStyleImage;

// ITK
#include "itkImage.h"
#include "itkVectorImage.h"

// Qt
#include <QMainWindow>

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

signals:
  void signal_TargetPatchMoved(const itk::ImageRegion<2>&);
  void signal_SourcePatchMoved(const itk::ImageRegion<2>&);

public slots:

  void on_actionOpenImage_activated();

  void on_actionHelp_activated();
  void on_actionQuit_activated();

  void on_txtPatchRadius_returnPressed();

  void RefreshSlot();

private:

  void UpdatePatches();

  unsigned int GetPatchRadius();

  void showEvent(QShowEvent* event);

  void OpenImage(const std::string& filename);

  void GetPatchSize();

  // Allow us to interact with the objects as we would like.
  vtkSmartPointer<vtkInteractorStyleImage> InteractorStyle;

  // Display the image appropriately
  ITKVTKCamera itkvtkCamera;

  vtkSmartPointer<vtkRenderer> Renderer;

  // Image display
  Layer ImageLayer;

  // The data that the user loads
  ImageType::Pointer Image;

  itk::Size<2> PatchSize;

};

#endif // InteractivePatchProjectionWidget_H
