#!/bin/sh -eux
rootfs_dir="./alpine-root"
filename="alpine-minirootfs.tar.gz"
rootfs_url="http://dl-cdn.alpinelinux.org/alpine/v3.8/releases/x86_64/alpine-minirootfs-3.8.2-x86_64.tar.gz"

mkdir ${rootfs_dir}
curl ${rootfs_url} > ${rootfs_dir}/${filename}
cd ${rootfs_dir}
sudo tar -zxvf ./${filename}
cd ../
sudo chown -R ${USER} ${rootfs_dir}
sudo chgrp -R ${USER} ${rootfs_dir}
rm ${rootfs_dir}/${filename}

