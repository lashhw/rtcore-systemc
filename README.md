# rtcore-systemc
Modeling RT Core with SystemC

## Clone
```shell
git clone --recursive https://github.com/lashhw/rtcore-systemc.git
```

## Build
```shell
mkdir build
cd build
cmake ..
cmake --build . --target tb_top
```

## Usage
Place [kitchen.ply](https://drive.google.com/file/d/1a9jkAWW94ez3sJRQ8pJa9crPwDPc2m5P/view?usp=sharing) and [ray_queries.bin](https://drive.google.com/file/d/125fEeSPRotU1eA4HJ4FtO8Qh9pM6q_gk/view?usp=sharing) in `build` directory.
```shell
./tb_top
```