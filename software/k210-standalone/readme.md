
## Cmake 文档修改

`CMakeLists.txt` 替换官方sdk目录下的 `CMakeLists.txt`

## 运行cmake（位于build目录下）

```bash
cmake .\..\..\..\kendryte-standalone-sdk -DPROJ=xxxxxx -G "MinGW Makefiles"
```

## 编译（位于build目录下）

```bash
make -j
```

## 烧录（位于build目录下）
```bash
kflash -p COM3 -b 2000000 -B bit .\xxxxxx.bin
```