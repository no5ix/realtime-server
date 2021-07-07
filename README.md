一个轻量级游戏服务器框架


# python version

python 3.8.8


# 要点

- 业务层基于ECS框架来做开发, 继承实体基类与组件基类即可
- 基于msgpack的RPC框架, 支持 ip地址直接call以及配合ECS的remote虚拟实体/组件直接call
- 基于asyncio异步IO的协程业务层支持, 可实现类似 `result = await rpc_call()` 的效果
    - 实现了协程池, 封装成简洁的装饰器便于业务层调用
- 支持异步的TimedRotating日志模块
    - 自动根据日期时间切换日志文件
    - 支持协程对象的callback
    - 根据日志level改变颜色, 方便查询
    - 报trace可打印堆栈与`locals`
    - 对于 warning 以上的日志级别直接对Pycharm提供文件跳转支持
- 支持1:N模型的定时器模块, 避免覆盖同一个key的易错点 
    - 可以重复使用一个key, 并不会冲掉之前key的timer, 但是当调用`cancel_timer`的时候, 会一次性全部cancel掉所有
- 制作了增强型json解析器, 支持注释/自动去除逗号/变量宏
- 增量式热更新reload模块
- 基于etcd的 服务注册 / TTL / 服务发现 / 负载均衡 / 上报负载 / Watch机制 一体化
- 断线重连
- 基于MongoDB的数据落地模块
- client端的模拟与自动化测试配套
- 大厅服务器的前置网关gate服务器, 负责压缩/解压, 加密/解密数据以及鉴权


# ToDo List

- Unity基于pythonnet来引入python实现热更或业务编写


# 架构


### 客户端角度

```puml
actor game_player_client as c
database S3

rectangle gs [
    <b>游戏服(gamesvr)
    ....
    Python with asiocore
    ----
    Avatar
    部分Center, Stub
]

rectangle bs [
    <b>战斗服(battlesvr)
    ....
    Python with asiocore
    ----
    Entity: Battle, PuppetBindEntity
    LocalEntity: AvatarPuppet, Flyer, ...
]

rectangle ms [
    <b>微服务(mssvr)
    ....
    Python with gevent
    ----
    AvatarService, MatchService, BattleService, ...
]

rectangle api [
    <b>API Gateway
    ....
    网关
    ----
    外网用户访问https://g90agw.nie.netease.com
    内网用户访问http://g90agw-in.nie.netease.com
]

rectangle api_service [
    <b> API Service
    ....
    Golang HTTP Server in Symphony / ECS
    ----
    like, login, s3, ...
]

c <--> gs: TCP
c <--> bs: KCP/TCP
c <--> api: https
c --> S3: http

gs <--> ms
bs <--> ms
gs <--> bs
gs --> api: http/https
bs --> api: http/https
api <--> api_service
api_service --> S3
```

关系图如下:  
```puml
sprite $comp jar:archimate/component
sprite $service jar:archimate/service
sprite $server jar:archimate/device
sprite $client jar:archimate/actor

archimate #Physical Client as C <<actor>>
archimate #Physical Server as S <<device>>

archimate #Technology Ocean as DB <<service>>
archimate #Technology S3 <<service>>
archimate #Technology 计费 as pay <<service>>
archimate #Technology "API Gateway" as GW <<service>>

archimate #Application "g90/login-server" as SLogin <<component>>
archimate #Application "g68/rank" as SRank <<component>>
archimate #Application "g90/screen-bullet" as SBullet <<component>>
archimate #Application "g90/s3" as SS3 <<component>>
archimate #Application "g90/payment-server" as SPay <<component>>

C -down-> GW: HTTPS
GW -down-> SLogin: /auth
GW -down-> SBullet: /send
GW -down-> SRank: /get_rank

SRank <-down- S: Update Rank
SS3 <-down- S: Upload/List/Delete
SPay -down-> S: call online avt
SPay -down-> DB: insert doc into messages
S <-right-> DB

C --> S3: HTTP
SS3 -up-> S3: Upload/List/Delete

pay -down-> SPay: /ship

legend left
图例：
<$comp>: API service
<$service>: 外部服务
<$server>: 游戏服务器
<$client>: 客户端
==
注：服务器与API service之间的通信，同样走API Gateway
endlegend
```

### 周边服务角度

```puml
database Ocean as mongo
database redis [
    Jetis
    ....
    ElastiCache
]
database S3
database etcd

rectangle gs [
    <b>游戏服(gamesvr)
    ....
    Python with asiocore
    ----
    Avatar
    部分Center, Stub
    ----
    gamemanager
    dbmanager
    gate
    game
]

rectangle bs [
    <b>战斗服(battlesvr)
    ....
    Python with asiocore
    ----
    Entity: Battle, PuppetBindEntity
    LocalEntity: AvatarPuppet, Flyer, ...
    ----
    gamemanager
    dbmanager
    gate
    battle
]

rectangle ms [
    <b>微服务(mssvr)
    ....
    Python with gevent
    ----
    AvatarService, MatchService, BattleService, ...
    ----
    service_gate
    service
]

rectangle api [
    <b>API Service
    ....
    Golang HTTP Server in Symphony / ECS
    ----
    rank, login, s3, ...
]

api --> S3
api --> mongo
api --> redis

gs --> mongo
bs --> mongo
ms --> mongo
gs --> etcd
bs --> etcd
ms --> etcd
ms --> redis
```


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
- [ ] 打通 cli-gate-lobby-battle

- [ ] tick loop
<!-- - [ ] dispatcher_service -->
- [ ] 大厅服务器通知战斗服务器相关puppet的信息已经加密令牌, 约定通信协议
- [ ] 客户端拿着令牌来和战斗服务器连接并交互
<!-- - [ ] 各个战斗服务器之间的协同center stub, center掉了, stub尝试重连center的逻辑 -->
- [ ] 加上type hint
- [ ] 安全关闭服务器的时候关闭各种conn和server以及清理各种数据和落地
- [ ] lobby_gate
- [ ] lobby_server
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


# 代码规范

使用const变量之时不建议使用from ... import ... , 因为
> Don't use a from import unless the variable is intended to be a constant. from shared_stuff import a would create a new a variable initialized to whatever shared_stuff.a referred to at the time of the import, and this new a variable would not be affected by assignments to shared_stuff.a.

这样会影响incremental_reload, 而用reload_all的话则会卡顿一阵
