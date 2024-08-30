# 依赖项
1. 请确保系统中包含`gflags` `glog` `gtest` 库。

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
./build/delta --config="config.toml"
```