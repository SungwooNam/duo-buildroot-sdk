
## char device
trying DMA transfer 
...

```sh
[root@milkv-duo]/mnt# insmod army.ko
[root@milkv-duo]/mnt# dmesg
[ 2360.771303] army_dma_class army_dma : loading ... 
[ 2360.771327] army_dma : registed as major: 243, minor: 1
...
[root@milkv-duo]/mnt# echo "Hello, World!" >/dev/army_dma
-sh: can't create /dev/army_dma: No error information

[root@milkv-duo]/mnt# dmesg
[ 2360.771303] army_dma_class army_dma : loading ... 
[ 2360.771327] army_dma : registed as major: 243, minor: 1
[ 2488.791380] army_dma : opening
[ 2488.791392] DMA Test: Opening device
[ 2488.791401] DMA Test: Failed to request DMA channel
```
