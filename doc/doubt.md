## <center>目前不确定的问题点</center>

#### 1.关于ragel和http-parser
**2024/6/16   sylar bilibili 72节 有一个问题**
```cpp
http11_parser.rl  
这个文件中  exec处
parser->nread = 0;
parser->mark = 0;
parser->field_len = 0;
parser->field_start = 0;
初始化问题，
重新生成.cc 文件，
ragel -G2 -C http11_parser.rl -o http11_parser.rl.cc


另外修改了http_parser.cc文件中两处parser->setError(1002);
注释掉了parser->setError(1002);
这里后续可以不修改，github中sylar也注释了


另外，弹幕提供一个方法应该也可用
在http11_parser.rl  
这个文件中exec处，将parser->field_len = 0; 修改。
修改为parser->field_len = len;
当天测试是没问题，后续可以重试。
```
