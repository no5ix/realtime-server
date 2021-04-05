# version

python 3.8.8


# todo list

<!-- - [ ] reload all/rpc func而不需要rpc_impl, 给reload_impl加上log, test"from test_const import SD_STR" -->
<!-- - [ ] puppet -->
<!-- - [ ] avatar -->
- [ ] 等回调的逻辑的wrapper
- [ ] dungeon
<!-- - [ ] reload 支持 `rpc_func`装饰器的增删 -->
- [ ] 启动脚本
- [ ] 手动心跳
- [ ] kcp connection 共用心跳模块
- [ ] battle_server
- [ ] lobby_server
- [ ] game_mgr process for forwarding/reloading
- [ ] game_mgr_client to exec game_script
- [ ] battle_service
- [ ] db manager
- [ ] lobby_gate
- [ ] 
- [ ] base on etcd distributed lock

- [ ] login service
- [ ] rename
- [ ] 信号处理
- [ ] keyboardinterrupt
<!-- - [ ] exception 以及 各种抛出 -->
<!-- - [ ] timer with key and cancel -->
- [ ] tick loop
<!-- - [ ] etcd -->
<!-- - [ ] server call cli -->
<!-- - [ ] rpc_method装饰器的参数不一定要是tuple -->
- [ ] 加上type hint
- [ ] 配表导表工具
<!-- - [  ] 日志 -->
- [ ] 加密
- [ ] 压缩
- [ ] db
- [ ] test
- [ ] 大厅服务器通知战斗服务器相关puppet的信息已经加密令牌, 约定通信协议
- [ ] 客户端拿着令牌来和战斗服务器连接并交互
- [ ] 各个战斗服务器之间的协同center stub, center掉了, stub尝试重连center的逻辑
- [ ] 录像回放, 直播
<!-- - [ ] 配置json解析与初始化 -->
- [ ] 断线重连
- [ ] 安全关闭服务器的时候关闭各种conn和server以及清理各种数据和落地


# 代码规范

除了内置标准库或者第三方库的变量或者函数, 要使用const变量之时不建议使用from ... import ... , 因为
> Don't use a from import unless the variable is intended to be a constant. from shared_stuff import a would create a new a variable initialized to whatever shared_stuff.a referred to at the time of the import, and this new a variable would not be affected by assignments to shared_stuff.a.

这样会影响incremental_reload, 而用reload_all的话则会卡顿一阵
