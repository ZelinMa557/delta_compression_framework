# 依赖项
1. 请确保系统中包含`gflags` `glog` `gtest` 库。
2. 请安装`https://github.com/google/crc32c`库，包括头文件和动态连接库。
如果安装后，在运行时报错找不到crc32c库，请在系统中寻找libcrc32c.so，找到后将相关文件复制到/usr/lib或/usr/lib64等目录下。

# 测试数据
执行以下代码，获取19个不同版本的glib源码的tar包:
```
bash down_load_test_data.sh
```

# 编译运行
常规的cmake项目构建方式：
```
mkdir build
cd build
cmake ..
make
cd ..
```

运行：
```
./build/delta --config finesse.toml
./build/delta --config odess.toml
```