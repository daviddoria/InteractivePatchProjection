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

#include <QApplication>
#include <QCleanlooksStyle>

#include "InteractivePatchProjectionWidget.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  QApplication::setStyle(new QCleanlooksStyle);

  InteractivePatchProjectionWidget* interactivePatchProjectionWidget = NULL;

  if(argc == 1)
  {
    interactivePatchProjectionWidget = new InteractivePatchProjectionWidget;
  }
  else if(argc == 2)
  {
    std::string imageFileName = argv[1];

    std::cout << "imageFileName: " << imageFileName << std::endl;

    interactivePatchProjectionWidget = new InteractivePatchProjectionWidget(imageFileName);
  }
  else
  {
    std::cerr << "Input arguments invalid!" << std::endl;
    for(int i = 1; i < argc; ++i)
    {
      std::cerr << argv[i] << " ";
    }
    std::cerr << std::endl;
    return EXIT_FAILURE;
  }

  interactivePatchProjectionWidget->show();

  return app.exec();
}
