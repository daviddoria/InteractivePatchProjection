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

#ifndef PIXMAPDELEGATE_H
#define PIXMAPDELEGATE_H

#include <QStyledItemDelegate>

class PixmapDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:

  PixmapDelegate();

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

  void SetPadding(const unsigned int padding);

  QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const ;

private:
  unsigned int Padding; // This padding will be placed around the entire image in the cell
};

#endif
