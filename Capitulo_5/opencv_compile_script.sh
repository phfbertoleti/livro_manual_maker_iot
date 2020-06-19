#!/bin/bash

echo ""
echo "-----------------------------------------------"
echo "Instalacao de pacotes necessarios para o OpenCV"
echo "-----------------------------------------------"
echo ""

sudo apt-get install -y build-essential cmake pkg-config
sudo apt-get install -y libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
sudo apt-get install -y libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get install -y libxvidcore-dev libx264-dev
sudo apt-get install -y libgtk2.0-dev
sudo apt-get install -y libatlas-base-dev gfortran
sudo apt-get install -y python3-dev python3-pip

echo ""
echo "----------------------------------------"
echo "OpenCV 3.1.0 - download do codigo-fonte "
echo "----------------------------------------"
echo ""

cd ~
wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.1.0.zip
unzip opencv.zip

wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.1.0.zip
unzip opencv_contrib.zip

echo ""
echo "-------------------------------------"
echo "Instalando numpy e Scipy (Python3)..."
echo "-------------------------------------"
echo ""


sudo pip3 install numpy scipy

echo ""
echo "--------------------------"
echo "Compilacao do OpenCV 3.1.0"
echo "--------------------------"
echo ""

cd ~/opencv-3.1.0/
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D ENABLE_PRECOMPILED_HEADERS=OFF \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_PYTHON_EXAMPLES=ON \
    -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib-3.1.0/modules \
    -D BUILD_EXAMPLES=ON ..


make -j4

echo ""
echo "---------------------------"
echo "Instalando OpenCV 3.1.0..."
echo "---------------------------"
echo ""

sudo make install

echo ""
echo "-------------------------"
echo "Finalizando instalacao..."
echo "-------------------------"
echo ""

sudo ldconfig
