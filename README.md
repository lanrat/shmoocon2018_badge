# ShmooCon 2018 Badge Code

Be sure to short the solder pads at the bottom of the badge when flashing.

### Backup 

```
esptool.py --port /dev/ttyUSB0 read_flash 0x00000 0x100000 badge.bin
```


### Flash

```
esptool.py write_flash 0x000000 badge.bin
esptool.py write_flash --verify 0x000000 badge.bin 
```