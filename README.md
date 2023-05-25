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
cmake --build . --target tb_top --config Release
```

## Usage
```shell
./tb_top MODEL_PLY RAY_QUERIES [LP_L1C_ENTRIES] [HP_L1C_ENTRIES] [IST_L1C_ENTRIES] [T_TRAV_HIGH] [T_TRAV_LOW]
```
Example: [kitchen.ply](https://drive.google.com/file/d/1a9jkAWW94ez3sJRQ8pJa9crPwDPc2m5P/view?usp=sharing), [kitchen.bin](https://drive.google.com/file/d/125fEeSPRotU1eA4HJ4FtO8Qh9pM6q_gk/view?usp=sharing)
