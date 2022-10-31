# 多个MBTiles合并

<div align='right' ><a href = "http://www.tianzhixiang.com.cn" target="_blank">北京天智祥信息科技有限公司</a></div>

### 修订记录

|    日期    | 版本  | 描述 |  作者  |
| :--------: | :---: | :--: | :----: |
| 2022-10-30 | 1.0.1 |      | 窦立阳 |

### 目录
[TOC]

<div STYLE="page-break-after: always;"></div>

### 测试环境

```
windows 10
studio studio 2019
vcpkg 
第三方依赖库 vcpkg install jsoncpp protozero zlib sqlite3 msinttypes
jsoncpp: 解析param.json 参数
protozero: mbtile 解压缩
zlib: mbtile采用的压缩方式
sqlite3: mbtile 读写
msinttypes: stdint.h and inttypes.h 合集,dtoa_milo.h需要

说明: 源码 mapbox_tile_sets.hpp mapbox_tile_sets.cpp dtoa_milo.h 是从 tippecanoe [https://github.com/mapbox/tippecanoe] 项目中摘的一部分功能
```



### 使用方法

param.json

```json
{
  "task_num": 1, // 任务数量 暂时没用
  "thread_limit": 16, // 最大线程数量限制
  "tasks": [
    {
        // 需要被合并的mbtiles, 每个mbtiles同样可以包含多个layer
      "inputs": [ "../../../data/poly_shi_s5000.mbtiles", "../../../data/line_50river_s5000.mbtiles" ],
        // 合并后的mbtiles的输出路径, 如果这个目录下存在这个文件,会被删除; 如果这个文件被打开,或者有其他占用,程序会运行出错退出.
      "output": "../../../merge/poly_shi_line_river.mbtiles"
    }
      // ... 添加其他任务
  ]
}
```

param.json与程序放在同一目录下, 编辑param.json,然后运行程序,如果有问题检查param.json以及其参数内容的实际路径.

### 其他

```
TODO:
	1. 有些可能出错的地方需要加保护
	2. 目前图像可能不对, 需要确认多个图层叠加的mbtiles应该是什么样的
	3. 使用多线程后还是慢
	4. 尝试添加 UI 界面, 方便操作
```



