## 示例目录结构
```
demo_project/
├── CMakeLists.txt          ← 
├── main.cpp
├── utils.cpp
├── utils.h
├── module_a/
│   ├── a1.cpp
│   ├── a1.h
│   └── legacy/              ← 会被排除
│       └── old.cpp          ← 会被排除
├── module_b/
│   ├── b1.cpp
│   ├── b1.h
│   └── test/                ← 会被排除
│       └── test_b.cpp       ← 会被排除
└── build/                  ← 会被排除
    └── ...
```

## 编译方法
```
mkdir build
cd build
cmake ..
make
```
