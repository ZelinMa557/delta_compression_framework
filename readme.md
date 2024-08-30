本项目是一个差值压缩原型系统，实现了N-Transform, Finesse, Odess, Palantir等多种差值压缩方法。该系统将差值压缩流程分为分块、去重、特征计算、相似块检测、差值编码与数据块写入五个阶段，每个阶段之间采用流水线并行。

# 测试数据
执行以下代码，获取20个不同版本的glib源码的tar包、20个不同版本的linux源码的tar包、20个不同版本的postgresql源码的tar包、8个不同版本的gcc源码的tar包:
```
bash down_load_test_data.sh
```

# 编译运行
请确保系统中包含`gflags` `glog` `gtest` 库。  
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