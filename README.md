# version

python 3.8.8


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
- [ ] tick loop
- [ ] dispatcher_service
- [ ] 大厅服务器通知战斗服务器相关puppet的信息已经加密令牌, 约定通信协议
- [ ] 客户端拿着令牌来和战斗服务器连接并交互
<!-- - [ ] 各个战斗服务器之间的协同center stub, center掉了, stub尝试重连center的逻辑 -->
- [ ] 加上type hint
- [ ] 安全关闭服务器的时候关闭各种conn和server以及清理各种数据和落地
- [ ] lobby_gate
- [ ] lobby_server
- [ ] 鉴权


- [ ] base on etcd distributed lock


- [ ] kcp connection 共用心跳模块
- [ ] game_mgr process for forwarding/reloading
- [ ] game_mgr_client to exec game_script
- [ ] db manager

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
