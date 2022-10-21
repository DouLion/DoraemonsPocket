# Protobuf写文件 c++

### 0. 测试环境

```
windows 10
visual studio 2019
vcpkg 
第三方库  vcpkg install protozero jsoncpp
```



### 1. 创建 序列化类文件

##### 1.1编辑.proto文件

```protobuf
syntax = "proto2";
// proto3 需要 把 required 删除
option optimize_for = LITE_RUNTIME;
package topy_resp;

// 支持引包的方式  参照:ttps://github.com/tomi77/protobuf-gis/tree/master/gis/protobuf , 这里为了测试方便, 放到一个文件里面

message RegionResp {
	message Header{
		required string return_msg = 1;
		required int32 return_code = 2;
	}

	required Header header = 1;

	message Body 
	{
		message Point2D {
			required float x = 1;
			required float y = 2;
		}
		
		message LineString2D {
			repeated Point2D points = 1;
		}
		
		message Polygon {
			repeated LineString2D line_string = 1;
		}
		
		repeated Polygon coordinates = 1;
		required string id = 2;
		message Station {
			required string code = 1;
			required float lgtd = 2;
			required float lttd = 3;
			required string name = 4;
			required string type = 5;
			
		}
		repeated Station stations = 3;
	}
	
	required Body body = 2;
}


```

##### 1.2 创建类文件

安装protozero 后 ${vcpkg_root}/installed/x64-windows/tools/protobuf文件夹下会有 protoc.exe,将其添加到 ${PATH}

```bash
protoc -I=./ --cpp_out=./ ./ee.proto
```

得到 ee.pb.h和ee.pb.cc文件, 在项目中包含这两个文件, 然后就是正常c++ 使用源码的流程



### 2. 数据说明

```
此示例中,是将test.json文件转化为 test_region.pbf格式, 有个需要注意的地方, ifstream 和 FILE 读写文件好像有大端序 和 小端序的区别,后面需要验证
```

