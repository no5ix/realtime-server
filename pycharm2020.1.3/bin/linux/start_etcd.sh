./etcd/etcd --name etcd0 --initial-advertise-peer-urls http://127.0.0.1:2380 \
  --listen-client-urls http://127.0.0.1:2379 \
  --advertise-client-urls http://127.0.0.1:2379 \
  --initial-cluster-token etcd-cluster-0 \
  --initial-cluster-state new


 ./etcd/etcd --name etcd1 --initial-advertise-peer-urls http://127.0.0.1:2382 \
  --listen-client-urls http://127.0.0.1:2381 \
  --advertise-client-urls http://127.0.0.1:2381 \
  --initial-cluster-token etcd-cluster-1 \
  --initial-cluster-state new
