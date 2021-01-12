## DTS Net

网络协议栈实践

### 说明

- include目录：各模块接口其BSD Socket API

- ether：Ethernet模块

- arp：ARP拆包组包模块

- ether_arp: Ethernet-IP的ARP逻辑模块

- ip：IP模块

- icmp：ICMP模块

- udp：UDP模块

- dblk：数据块（数据结构）模块，用于传递在各层之间的数据。

### 如何使用

1. 实现`dts_net_mem.h`中的接口，可参考`dts.net.mem.impl`。

2. 绑定驱动，如：

    ```C
    ether_t eth0 = {
        .interface = (void*)&enc28j60,
        .send = (size_t(*)(void *,uint8_t*,size_t))enc28j60_send,
        .recv = (size_t(*)(void *,uint8_t*,size_t))enc28j60_recv,
        .mac_address = { 0x68, 0x19, 0x78, 0x11, 0x12, 0x34 },
    };
    ```
3. TODO

### 备注

外部需提供：

- 配置，`dts_net_conf.h`

- 内存相关接口，见`dts_net_mem.h`，实现参考`dts.net.mem.impl`文件夹下的内容

- 与底层驱动交互

    1. 提供ether实例，并绑定驱动接口。

    ```
    ether_t eth0 = {
        .interface = (void*)&enc28j60,
        .send = (size_t(*)(void *,uint8_t*,size_t))enc28j60_send,
        .recv = (size_t(*)(void *,uint8_t*,size_t))enc28j60_recv,
        .mac_address = { 0x68, 0x19, 0x78, 0x11, 0x12, 0x34 },
    };
    ```

    2. 硬件收到数据时，调用`ether_ll_recv`，用ether_frame_t指定缓冲区。ether_ll_recv内部会调用ether->recv读取数据。

    ```
    memset(buff, 0, 2048);
    frame.data = buff;
    frame.data_size = 2048;
    ether_ll_recv(&eth0, &frame);
    ```

- 对dts.datastruct的依赖

    目前仅依赖dts.datastruct.list

- 需要timer及rtc功能（目前还没有）