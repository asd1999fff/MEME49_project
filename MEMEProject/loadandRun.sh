sudo insmod module/DHT22/dht22.ko 
echo 'Driver: Load DHT22!'

sudo insmod module/MQ7/MQ7.ko 
echo 'Driver: Load MQ7!'

sudo insmod module/HW_508/HW_508.ko 
echo 'Driver: Load HW_508!'

sudo insmod module/White_LED/whiteled.ko 
echo 'Driver: Load White LED!'

sudo system/main &
echo 'Sysyem: Run!'
sudo system/camera 