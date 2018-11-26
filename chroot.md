# dockerを使ってイメージを展開
```
image_dir="./alpine-test"
mkdir ${image_dir}
cd ${image_dir}
sudo docker pull alpine
container_id=$(sudo docker create alpine)
sudo docker export ${container_id} > ./alpine.tar
tar xvf ./alpine.tar
rm alpine.tar
``

# 実行
```
sudo unshare -pf chroot ./alpine-test /bin/sh
mount -t proc proc /proc
mount -t devpts devpts /dev/pts
```

