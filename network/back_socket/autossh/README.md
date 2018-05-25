/*
 * ssh对硬件/系统的要求很低,利用ssh做端口转发实用性广,几乎任何上Linux系统都自带ssh.
 * 
 * issue:
 * 1.由于在服务器端把ServerAliveInterval和ServerAliveCountMax写入sshd配置文件,会导致不安全,
 * 2.ssh长时间不用,路由会阻断ssh的长连接
 * 3.网络故障导致的网络中断ssh连接
 * 
 * 原理:
 * 可以通过服务器的反馈信息来,自动判断连接/端口是否有效,是否需要重新连接
 * 1. Connection refused 服务器没有监听目标端口, 可以reverse ssh
 * 2. forwarding failed  端口被占用, 需要关闭进程,然后尝试连接,若返回Connection reset则可以reverse ssh
 * 3. Connection reset   服务器与客户端网络断开,服务器没有收到FIN , 再次连接可成功
 * 
 * create data :2018-04-18 18:21:21 by Eason
 */