

@title ETCD_Cluster
@ECHO off


@REM rmdir /S /Q default.etcd
@REM rmdir /S /Q etcd0.etcd
rmdir /S /Q infra0.etcd
rmdir /S /Q infra1.etcd
rmdir /S /Q infra2.etcd
@REM rmdir /S /Q etcd1.etcd

@REM etcd\etcd.exe --name etcd0 --initial-advertise-peer-urls http://192.168.82.177:2380 --listen-client-urls http://192.168.82.177:2379 --advertise-client-urls http://192.168.82.177:2379 --initial-cluster-token etcd-cluster-0 --initial-cluster-state new
@REM etcd\etcd.exe --name etcd1 --initial-advertise-peer-urls http://192.168.82.177:2382 --listen-client-urls http://192.168.82.177:2381 --advertise-client-urls http://192.168.82.177:2381 --initial-cluster-token etcd-cluster-1 --initial-cluster-state new

@REM etcd\etcd.exe --name etcd0 ^
@REM     --initial-advertise-peer-urls http://192.168.82.177:2380 ^
@REM     --listen-peer-urls http://192.168.82.177:2380 ^
@REM     --listen-client-urls http://192.168.82.177:2379 ^
@REM     --advertise-client-urls http://192.168.82.177:2379 ^
@REM     --initial-cluster-token etcd-cluster-0 ^
@REM     --initial-cluster-state new

@REM set MY_IP=192.168.82.177
@REM set MY_IP=192.168.1.4
set MY_IP=127.0.0.1


start /b etcd\etcd.exe --name infra0 ^
    --initial-advertise-peer-urls http://%MY_IP%:2380 ^
    --listen-peer-urls http://%MY_IP%:2380 ^
    --listen-client-urls http://%MY_IP%:2379 ^
    --advertise-client-urls http://%MY_IP%:2379 ^
    --initial-cluster-token etcd-cluster-1 ^
    --initial-cluster infra0=http://%MY_IP%:2380,infra1=http://%MY_IP%:2382,infra2=http://%MY_IP%:2384 ^
    --initial-cluster-state new

start /b etcd\etcd.exe --name infra1 ^
    --initial-advertise-peer-urls http://%MY_IP%:2382 ^
    --listen-peer-urls http://%MY_IP%:2382 ^
    --listen-client-urls http://%MY_IP%:2381 ^
    --advertise-client-urls http://%MY_IP%:2381 ^
    --initial-cluster-token etcd-cluster-1 ^
    --initial-cluster infra0=http://%MY_IP%:2380,infra1=http://%MY_IP%:2382,infra2=http://%MY_IP%:2384 ^
    --initial-cluster-state new

start /b etcd\etcd.exe --name infra2 ^
    --initial-advertise-peer-urls http://%MY_IP%:2384 ^
    --listen-peer-urls http://%MY_IP%:2384 ^
    --listen-client-urls http://%MY_IP%:2383 ^
    --advertise-client-urls http://%MY_IP%:2383 ^
    --initial-cluster-token etcd-cluster-1 ^
    --initial-cluster infra0=http://%MY_IP%:2380,infra1=http://%MY_IP%:2382,infra2=http://%MY_IP%:2384 ^
    --initial-cluster-state new



@REM start /b etcd\etcd.exe --name infra0 ^
@REM     --initial-advertise-peer-urls http://%MY_IP%:2380 ^
@REM     --listen-peer-urls http://%MY_IP%:2380 ^
@REM     --listen-client-urls http://%MY_IP%:2379,http://127.0.0.1:2379 ^
@REM     --advertise-client-urls http://%MY_IP%:2379 ^
@REM     --initial-cluster-token etcd-cluster-1 ^
@REM     --initial-cluster infra0=http://%MY_IP%:2380,infra1=http://%MY_IP%:2382,infra2=http://%MY_IP%:2384 ^
@REM     --initial-cluster-state new

@REM start /b etcd\etcd.exe --name infra1 ^
@REM     --initial-advertise-peer-urls http://%MY_IP%:2382 ^
@REM     --listen-peer-urls http://%MY_IP%:2382 ^
@REM     --listen-client-urls http://%MY_IP%:2381,http://127.0.0.1:2381 ^
@REM     --advertise-client-urls http://%MY_IP%:2381 ^
@REM     --initial-cluster-token etcd-cluster-1 ^
@REM     --initial-cluster infra0=http://%MY_IP%:2380,infra1=http://%MY_IP%:2382,infra2=http://%MY_IP%:2384 ^
@REM     --initial-cluster-state new

@REM start /b etcd\etcd.exe --name infra2 ^
@REM     --initial-advertise-peer-urls http://%MY_IP%:2384 ^
@REM     --listen-peer-urls http://%MY_IP%:2384 ^
@REM     --listen-client-urls http://%MY_IP%:2383,http://127.0.0.1:2383 ^
@REM     --advertise-client-urls http://%MY_IP%:2383 ^
@REM     --initial-cluster-token etcd-cluster-1 ^
@REM     --initial-cluster infra0=http://%MY_IP%:2380,infra1=http://%MY_IP%:2382,infra2=http://%MY_IP%:2384 ^
@REM     --initial-cluster-state new

color c1
pause
