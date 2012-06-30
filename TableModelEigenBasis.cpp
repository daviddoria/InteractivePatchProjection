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

#include "TableModelEigenBasis.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"
#include "PatchProjection/PatchProjection.h"

TableModelEigenBasis::TableModelEigenBasis(const Eigen::MatrixXd& eigenvectorMatrix, QObject * parent) :
    QAbstractTableModel(parent), PatchDisplaySize(20), EigenvectorMatrix(eigenvectorMatrix)
{
}

void TableModelEigenBasis::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags TableModelEigenBasis::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

int TableModelEigenBasis::rowCount(const QModelIndex& parent) const
{
  return 2;
}

int TableModelEigenBasis::columnCount(const QModelIndex& parent) const
{
  //std::cout << "There are " << this->EigenvectorMatrix.cols() << " cols" << std::endl;
  return this->EigenvectorMatrix.cols();
}

QVariant TableModelEigenBasis::data(const QModelIndex& index, int role) const
{
  //std::cout << "data() requested for " << index.row() << " " << index.column() << std::endl;

  if(role == Qt::DisplayRole && index.row() >= 0 && index.column() >= 0)
    {
    switch(index.row())
      {
      case 0:
        {
        return index.column();
        }
      case 1:
        {
        //std::cout << "Create basis image " << index.column() << "." << std::endl;
        //typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
        typedef itk::VectorImage<float, 2> ImageType;
        ImageType::Pointer image = ImageType::New();

        Eigen::VectorXd eigenColumn = this->EigenvectorMatrix.col(index.column());
        Eigen::VectorXd scaled = EigenHelpers::ScaleVector(eigenColumn,
                                                           0.0f, 255.0f);

        PatchProjection<Eigen::MatrixXd, Eigen::VectorXd>::UnvectorizePatch
          (scaled, image.GetPointer(), 3);

        QImage patchImage = ITKQtHelpers::GetQImageColor(image.GetPointer(),
                                                         image->GetLargestPossibleRegion());

        patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

        return QPixmap::fromImage(patchImage);
        }
      } // end switch

    } // end if DisplayRole

  return QVariant();
}

QVariant TableModelEigenBasis::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant returnValue;
  return returnValue;
}

void TableModelEigenBasis::Refresh()
{
  beginResetModel();
  endResetModel();
}

void TableModelEigenBasis::SetEigenvectorMatrix(const Eigen::MatrixXd& eigenvectorMatrix)
{
//   beginInsertRows();
//   beginInsertColumns();
  
  this->EigenvectorMatrix = eigenvectorMatrix;

//   endInsertRows();
//   endInsertColumns();
  
  Refresh();
  
}