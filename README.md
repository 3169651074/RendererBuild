# RendererBuild
一个简单的光线追踪器，使用GPU友好的数据结构进行重构，为移植到纯CUDA做准备

## Midifications
* 完全重构了数据结构，移除了所有继承关系和虚函数
* 将函数分为CPU端执行的和GPU端执行的。对于需要在GPU端执行的函数，移除所有智能指针，使用类型+索引的C风格数组代替std::vector
* 为了完成多态的功能，所有类型的数组都需要作为参数传递进主渲染函数，并使用switch语句根据类型属性采用对应的数组，再使用下标定位元素
* 这种架构方式相对于经典的面向对象设计，添加新类型的扩展性较差，但添加新功能的扩展性较好，只需要增加对应的函数即可
* 由于没有了虚函数和智能指针，此渲染器的性能略微高于面向对象设计的版本
* 将所有递归改为迭代：迭代式光线追踪主函数和迭代式BVH遍历函数。以适应GPU函数不能递归的限制

## Build
### Windows
克隆仓库后直接使用CMake编译即可

### Ubuntu/Debian
1. 使用系统包管理器安装SDL2依赖库和GCC
```
sudo apt update && sudo apt install libsdl2-dev libsdl2-image-dev gcc g++
```
2. 使用oneapi安装Intel OIDN降噪器  
安装必要工具
```
sudo apt update && sudo apt install -y gpg-agent wget
```

下载并添加 Intel GPG 公钥，这个密钥用于验证从 Intel 仓库下载的软件包的完整性
```
wget -O- https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB \ | gpg --dearmor | sudo tee /usr/share/keyrings/oneapi-archive-keyring.gpg > /dev/null
```

添加 OneAPI 仓库到 APT 源列表，会创建一个新的文件，告诉系统从哪里获取 OneAPI 软件包
```
echo "deb [signed-by=/usr/share/keyrings/oneapi-archive-keyring.gpg] https://apt.repos.intel.com/oneapi all main" | sudo tee /etc/apt/sources.list.d/oneAPI.list
```

让包管理器获取新的软件包并安装OIDN（安装全部工具：sudo apt install intel-basekit）
```
sudo apt update && sudo apt install intel-renderkit
```

配置运行时动态库查找路径（撤销修改：sudo rm /etc/ld.so.conf.d/oneapi.conf && sudo ldconfig）
```
echo "/opt/intel/oneapi/oidn/2.2/lib/" | sudo tee /etc/ld.so.conf.d/oneapi.conf && sudo ldconfig
```

然后使用CMake编译，同时传入CMake参数（需要替换为实际的安装路径，由于版本不同安装路径可能不同）
```
-DOpenImageDenoise_DIR=/opt/intel/oneapi/oidn/2.2/lib/cmake/OpenImageDenoise/
```

3. 设置环境变量（运行时需要）  
1.使用CLion运行
```
在右上角项目名 -> 下拉菜单 -> Edit Configuration -> Environment Variables中添加环境变量
1.在新终端中执行source /opt/intel/oneapi/setvars.sh
2.echo $LD_LIBRARY_PATH
3.将输出的字符串完整复制作为环境变量值
4.保存修改
```

2.在终端中运行（新开一个终端），第二条命令无需严格执行，在终端中运行编译好的可执行文件即可
```
1.source /opt/intel/oneapi/setvars.sh
2.cd bin && ./RendererBuild
```
