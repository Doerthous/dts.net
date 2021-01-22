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

- tcp: TCP模块（TO BE CONTINUE）

- dblk：数据块（数据结构）模块，用于传递在各层之间的数据。

### 如何使用

1. 实现`dts_net_mem.h`中的接口，可参考`dts.net.mem.impl`。

2. 绑定驱动，将ether模块与以太网网卡驱动绑定，如：

    ```C
    ether_t eth0 = {
        .interface = (void*)&enc28j60,
        .send = (size_t(*)(void *,uint8_t*,size_t))enc28j60_send,
        .recv = (size_t(*)(void *,uint8_t*,size_t))enc28j60_recv,
        .mac_address = { 0x68, 0x19, 0x78, 0x11, 0x12, 0x34 },
    };
    ```

3. 硬件收到数据时，调用`ether_ll_recv`，用ether_frame_t指定缓冲区。ether_ll_recv内部会调用ether->recv读取数据。

    ```
    memset(buff, 0, 2048);
    frame.data = buff;
    frame.data_size = 2048;
    ether_ll_recv(&eth0, &frame);
    ```

4. 绑定网络地址

    ```
    ip_t ip0 = {
        IP_IPv4_ADDR(10,0,0,2),
        IP_IPv4_MASK(255,255,255,0),
    };
	
	ether_arp_resgiter_pair(&ip0, &eth0);
    ```

5. 初始化及协议栈心跳

    ```
    // 初始化dts_net_mem模块
    dts_net_mem_init(); 

    ...

    // 周期性调用此函数（协议栈内部需要心跳）
    // 其频率需要在dts_net_conf.h中指定：
    // #define DTS_NET_SYS_HEARTBEAT_FREQ      1 // HZ
    dts_net_sys_heartbeat();
    ```

6. 配置，`dts_net_conf.h`

### 备注

- 外部依赖

    - 对dts.datastruct的依赖

        目前仅依赖dts.datastruct.list

    - 对dts.elib的依赖

        dts.elib.timer

    - 对dts.coroutine（v1.0）的依赖（仅test需要）

    - 可能需要rtc功能（目前还没有）