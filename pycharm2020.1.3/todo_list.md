# 架构图


```puml
actor game_player_client as cl
database MongoDB
database Redis
database etcd

rectangle hs [
    <b>HTTP微服务集群
    ....
    基于sanic开发的异步HTTP微服务类集群
    ----
    可基于此扩展实现登陆微服务、排队微服务、排行榜微服务等等
]

rectangle lg [
    <b>lobby_gate0, lobby_gate1, ...
    ----
    负责客户端到 lobby 服 之间的数据转发
]

rectangle lb [
    <b>lobby_0, lobby_1, ....
    ----
    负责游戏大厅逻辑的集群
]

rectangle bs [
    <b>battle_0, battle_1, ...
    ----
    承载战斗服逻辑的集群
]

rectangle ds[
    <b>调度逻辑类集群
    ....
    基于etcd的 服务注册 / TTL / 服务发现 / 负载均衡 / 上报负载 / Watch机制 的调度逻辑类集群
    ----
    可基于此扩展实现匹配类逻辑、全局广播类逻辑
]

cl <--> lg: TCP
lb <--> bs: TCP
lg <--> lb: TCP
cl <--> bs: TCP/RUDP
lb <--> ds: TCP
lg <--> ds: TCP
bs <--> ds: TCP

cl <--> hs: HTTPS
lb <--> hs: HTTP/HTTPS
bs <--> hs: HTTP/HTTPS

ds <--> Redis
hs <--> Redis
hs <--> MongoDB
lb <--> MongoDB

lb <--> etcd: HTTP/HTTPS
lg <--> etcd: HTTP/HTTPS
bs <--> etcd: HTTP/HTTPS
ds <--> etcd: HTTP/HTTPS
```


# 代码规范

使用const变量之时不建议使用from ... import ... , 因为
> Don't use a from import unless the variable is intended to be a constant. from shared_stuff import a would create a new a variable initialized to whatever shared_stuff.a referred to at the time of the import, and this new a variable would not be affected by assignments to shared_stuff.a.

这样会影响incremental_reload, 而用reload_all的话则会卡顿一阵


# todo list

<!-- - [ ] reload all/rpc func而不需要rpc_impl, 给reload_impl加上log, test"from test_const import SD_STR" -->
<!-- - [ ] puppet -->
<!-- - [ ] avatar -->
<!-- - [ ] 等回调的逻辑的wrapper -->
- [ ] dungeon
<!-- - [ ] reload 支持 `rpc_func`装饰器的增删 -->
- [ ] 启动脚本
<!-- - [ ] 手动心跳 -->
<!-- - [ ] 断线重连 -->
<!-- - [ ] battle_server -->
- [ ] base on etcd distributed lock
- [ ] kcp connection 共用心跳模块
<!-- - [ ] rudp 和tcp共存, 先rudp再tcp -->
<!-- - [ ] 服务器快速重启, 客户端应该要可以自动重连 -->
<!-- - [ ] 打通 cli-gate-lobby-battle -->

- [ ] tick loop
<!-- - [ ] dispatcher_service -->
- [ ] 大厅服务器通知战斗服务器相关puppet的信息已经加密令牌, 约定通信协议
- [ ] 客户端拿着令牌来和战斗服务器连接并交互
<!-- - [ ] 各个战斗服务器之间的协同center stub, center掉了, stub尝试重连center的逻辑 -->
- [ ] 加上type hint
- [ ] 安全关闭服务器的时候关闭各种conn和server以及清理各种数据和落地
<!-- - [ ] lobby_gate -->
<!-- - [ ] lobby_server -->
- [ ] 玩家离线存盘
- [ ] 鉴权
<!-- - [ ] bug: 当某些服务器已经下线, 但etcd 的ttl没处理好, 还在
- [ ] bug: 当某些服务器已经下线, 但`pick_lowest_load_service_addr`里的redis没有让某些服务器过期
  - [ ] fix: 用etcd来记录cpu load
- [ ] 服务器下线应该主动通知redis/etcd -->
- [ ] game_mgr process for forwarding/reloading
- [ ] game_mgr_client to exec game_script
- [ ] db manager
- [ ] quic

- [ ] login service
- [ ] rename
- [ ] 信号处理
- [ ] keyboardinterrupt
<!-- - [ ] exception 以及 各种抛出 -->
<!-- - [ ] timer with key and cancel -->
<!-- - [ ] etcd -->
<!-- - [ ] server call cli -->
<!-- - [ ] rpc_method装饰器的参数不一定要是tuple -->
- [ ] 配表导表工具
<!-- - [ ] 日志: 按日期滚动, 并输出到控制台并且按照日志等级有颜色区分, 可以多进程同时输出也可以默认另开线程输出 -->
- [ ] 日志输出到运营日志
- [ ] 加密
- [ ] 压缩
- [ ] db
- [ ] test
- [ ] 录像回放, 直播
<!-- - [ ] 配置json解析与初始化 -->
