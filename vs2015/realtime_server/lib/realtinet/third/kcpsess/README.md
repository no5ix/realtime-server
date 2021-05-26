
# QQ群

因为 KCP 官方群已经满了, 可以加群 496687140


# 轻量级的kcp会话实现-kcpsess

`kcpsess`真正实现了只需要随意写几行代码就可以用上kcp, 而无需烦心如何组织代码来适配kcp

- 只需包含 `kcpsess.h` 这一个头文件即可
- 只需调用 `KcpSession::Send` 和 `KcpSession::Recv` 和 `KcpSession::Update` 即可完成UDP的链接状态管理、会话控制、 RUDP协议调度

# kcpsess Example

- [realtime-server](https://github.com/no5ix/realtime-server) : A realtime dedicated game server ( FPS / MOBA ). 一个实时的专用游戏服务器.
- [realtime-server-ue4-demo](https://github.com/no5ix/realtime-server-ue4-demo) :  A UE4 State Synchronization demo for realtime-server. 为realtime-server而写的一个UE4状态同步demo, [Video Preview 视频演示](https://hulinhong.com)
- [TestKcpSessionServer.cpp](https://github.com/no5ix/kcpsess/blob/master/TestKcpSessionServer.cpp)
- [TestKcpSessionClient.cpp](https://github.com/no5ix/kcpsess/blob/master/TestKcpSessionClient.cpp)


# kcpsess Usage

the main loop was supposed as:

``` c++
GameInit()

// kcpsess init
KcpSession kcpClient(
    KcpSession::RoleTypeE,
    std::bind(udp_output, _1, _2),
    std::bind(udp_input),
    std::bind(timer));

While (!isGameOver) Do      // e.g:  A 30FPS Game

       while (KCPSESS.Recv(data, len))
       {
           if (len > 0)
           {
               Game.HandleRecvData(data, len)
           }
           else if (len < 0)
           {
               Game.HandleRecvError(len);
           }
       }
       KCPSESS.Send(data, len)
       KCPSESS.Update()
       Game.Logic()
       Game.Render()
       Wait(33ms)   // clock
End
```

The Recv/Send/Update functions of kcpsess are guaranteed to be non-blocking.
Please read [TestKcpSessionClient.cpp](https://github.com/no5ix/kcpsess/blob/master/TestKcpSessionClient.cpp) and [TestKcpSessionServer.cpp](https://github.com/no5ix/kcpsess/blob/master/TestKcpSessionServer.cpp) for some basic usage.


# kcpsess build & test

1. ` cmake . `
2. ` make `
3. ` ./ServerTestKcpSession `
4. ` ./ClientTestKcpSession `

# kcp源码注释