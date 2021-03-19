@title ETCD
@ECHO off

@REM rmdir /S /Q default.etcd
etcd\etcd.exe --name etcd0 --initial-advertise-peer-urls http://192.168.82.177:2380 --listen-client-urls http://192.168.82.177:2379 --advertise-client-urls http://192.168.82.177:2379 --initial-cluster-token etcd-cluster-0 --initial-cluster-state new
etcd\etcd.exe --name etcd1 --initial-advertise-peer-urls http://192.168.82.177:2382 --listen-client-urls http://192.168.82.177:2381 --advertise-client-urls http://192.168.82.177:2381 --initial-cluster-token etcd-cluster-1 --initial-cluster-state new
color c1
pause
