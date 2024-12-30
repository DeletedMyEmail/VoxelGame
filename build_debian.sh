sudo apt install libspdlog-dev -y
sudo apt install libglfw3-dev -y
sudo apt install cmake -y

mkdir build
cd build/
cmake ..
cmake --build .
