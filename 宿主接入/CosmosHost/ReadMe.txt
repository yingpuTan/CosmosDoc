
运行CosmosHostDemo程序需要下载cosmos引擎包

下载完成后调整代码
1.修改CCosmosHostApi.cpp：192，设置CosmosSDK.dll或libCosmosSDK.so路径
2.修改CCosmosHostApi.cpp：220，设置Cosmos.MainApp.exe路径
3.从cosmos引擎包中拷贝osslsigncode.dll放到宿主程序运行目录下

注意，所有Cosmos.Product.Sdk.h中的参数都需要传utf编码的格式