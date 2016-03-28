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

2. 依赖pylon
   (1) pylon的安装
       cd ~/pylon-5.0.0.1234-x86
       将pylon安装到了/usr/local下, 目录为/usr/local/pylon5
       sudo tar -C /usr/local -xzf pylonSDK*.tar.gz
       编译Sample, 由于pylon安装到了/usr/local/pylon5, 因此需要运行设置环境变量脚本, 如下:
       source /usr/local/pylon5/bin/pylon-setup-env.sh /usr/local/pylon5
       或者
       export PYLON_ROOT=/usr/local/pylon5
       然后编译Sample
       make
       执行./Grab/Grab
       
   (2) 连接和调试相机
       关闭防火墙: sudo systemctl status firewalld.service
       关闭reverse path filter: 
       编辑/etc/sysctl.conf文件, 
        net.ipv4.conf.all.rp_filter=0
        net.ipv4.conf.eth1.rp_filter=0
       根据实际情况而定。
       添加网络连接，在ipv4标签页中将"地址"改为"仅本地连接".

       
