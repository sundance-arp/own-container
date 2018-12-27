image_dir="./alpine-root"
mkdir ${image_dir}
cd ${image_dir}
sudo docker pull alpine
container_id=$(sudo docker create alpine)
sudo docker export ${container_id} > ./alpine.tar
sudo docker rm ${container_id}
tar xvf ./alpine.tar
rm alpine.tar

