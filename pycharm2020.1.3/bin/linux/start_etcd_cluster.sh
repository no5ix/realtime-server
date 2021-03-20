#!/bin/sh

rm -rf ./infra0.etcd
rm -rf ./infra1.etcd
rm -rf ./infra2.etcd

nohup ./etcd/etcd --name infra0 \
    --initial-advertise-peer-urls http://192.168.1.14:2380 \
    --listen-peer-urls http://192.168.1.14:2380 \
    --listen-client-urls http://192.168.1.14:2379,http://127.0.0.1:2379 \
    --advertise-client-urls http://192.168.1.14:2379 \
    --initial-cluster-token etcd-cluster-1 \
    --initial-cluster infra0=http://192.168.1.14:2380,infra1=http://192.168.1.14:2382,infra2=http://192.168.1.14:2384 \
    --initial-cluster-state new \
    > ./etcd0.log 2>&1 &

nohup ./etcd/etcd --name infra1 \
    --initial-advertise-peer-urls http://192.168.1.14:2382 \
    --listen-peer-urls http://192.168.1.14:2382 \
    --listen-client-urls http://192.168.1.14:2381,http://127.0.0.1:2381 \
    --advertise-client-urls http://192.168.1.14:2381 \
    --initial-cluster-token etcd-cluster-1 \
    --initial-cluster infra0=http://192.168.1.14:2380,infra1=http://192.168.1.14:2382,infra2=http://192.168.1.14:2384 \
    --initial-cluster-state new \
    > ./etcd1.log 2>&1 &

nohup ./etcd/etcd --name infra2 \
    --initial-advertise-peer-urls http://192.168.1.14:2384 \
    --listen-peer-urls http://192.168.1.14:2384 \
    --listen-client-urls http://192.168.1.14:2383,http://127.0.0.1:2383 \
    --advertise-client-urls http://192.168.1.14:2383 \
    --initial-cluster-token etcd-cluster-1 \
    --initial-cluster infra0=http://192.168.1.14:2380,infra1=http://192.168.1.14:2382,infra2=http://192.168.1.14:2384 \
    --initial-cluster-state new \
    > ./etcd2.log 2>&1 &
