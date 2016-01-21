# sieve
1. 依赖hiredis
   (1) hiredis的安装
       采用源码安装redis时，hiredis在源码目录下的deps/hiredis中
       cd deps/hiredis
       make
       生成libhiredis.so
       将libhiredis.so拷贝到/usr/local/lib下，将hiredis.h拷贝到/usr/local/include下

   (2) 使用hiredis
       编译时需要包含hiredis.h，编译选项需要指定 -I/usr/local/include
       链接时需要 -L/usr/local/lib -lhiredis 
