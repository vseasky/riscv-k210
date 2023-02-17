1、修改cpu1.main.c中如下定义
    DEFAULT_WLAN_SSID 为要连接的wifi名称
    DEFAULT_WLAN_PASSWORD 为要连接的wifi密码
    DEFAULT_SERVER_IP_ADDR 为PC机的IP地址
    DEFAULT_SERVER_PORT_ADDR 为通讯使用的网络端口

注：保证运行网络调试助手的机器(IP地址)和设备(WIFI)在同一网段内。
    建议使用笔记本作为热点，设备连接笔记本的wifi,在笔记本上运行网络调试助手。

2、编译并下载到设备中
3、运行网络调试助手，协议类型选择 TCP server,本地主机端口填写第1步中使用的端口，点击“打开”
4、设备上电，可以看到设备不断发送过来的“hello_world！”