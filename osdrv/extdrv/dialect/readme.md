
```sh
[root@milkv-duo]/mnt# insmod dialect.ko
[root@milkv-duo]/mnt# dmesg | tail -10
...
[  127.340488] I was assigned major number 243. To talk to
[  127.340501] the driver, create a dev file with
[  127.340510] 'mknod /dev/dialect c 243 0'.
[  127.340516] Try various minor numbers. Try to cat and echo to
[  127.340521] the device file.
...
[root@milkv-duo]/mnt# mknod /dev/dialect c 243 0
[root@milkv-duo]/mnt# echo "Hello, World!" > /dev/dialect
[root@milkv-duo]/mnt# cat /dev/dialect
Hello, World!
[root@milkv-duo]/mnt# rmmod dialect
[root@milkv-duo]/mnt# dmesg | tail -10
...
[  222.765088] Goodbye, world!
```