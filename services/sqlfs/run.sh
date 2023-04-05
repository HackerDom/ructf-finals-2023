go build .
umount /mnt/loop
./sqlfs /mnt/loop /home/user/loopbackfs
