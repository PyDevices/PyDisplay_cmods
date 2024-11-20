# pydisplay_cmods
MicroPython User C Modules for pydisplay

## WARNING
This is a work in progress and very much alpha quality.  Do not use in a production environment.

## Note on ESP32 partition tables
The partition table for the MicroPython ESP32 build is defined in one of the `partitions-*.csv` files in `micropython/ports/esp32`. The partition table is used to define the size and location of the partitions on the flash memory of the ESP32. The partition table is used by the `esptool.py` utility to flash the firmware to the ESP32.  Adding user c modules to your build may require a different partition table.  The partition table is defined in CSV format with the following columns:

```
# Name,   Type, SubType, Offset,  Size, Flags
```

The current files are:
- partitions-2MiB.csv
- partitions-4MiB.csv
- partitions-8MiB.csv
- partitions-16MiB.csv
- partitions-32MiB.csv
- partitions-4MiB-ota.csv
- partitions-16MiB-ota.csv
- partitions-32MiB-ota.csv

Here is an example of the `partitions-8MiB.csv` file:

```
# Notes: the offset of the partition table itself is set in
# $IDF_PATH/components/partition_table/Kconfig.projbuild.
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 0x1F0000,
vfs,      data, fat,     0x200000, 0x600000,
```
The bootloader and partition table itself occupy the space from 0x0 to 0x9000.  The nvs partition holds the non-volatile storage.  The phy_init partition holds the PHY initialization data.  We are concerned with the last two partitions: the factory and vfs partitions.  The factory partition holds the MicroPython firmware.  The vfs partition holds the file system.  In this case, the firmware may be up to 2MB (0x1F0000), and the filesystem is 6MB (0x600000).  In order to accommodate a larger firmware, we must reallocate space from the vfs to the factory partition.  For instance, to add 512KB (0x80000) to the factory partition, we would add 0x80000 to the size of the factory partition and the offset of the vfs partition, and subtract 0x80000 from the size of the vfs partition.  The new partition table would look like this:

```
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 0x270000,
vfs,      data, fat,     0x280000, 0x580000,
```

To check our work, th sum of the vfs offset and size should equal the size of the flash memory.  In this case, 0x280000 + 0x580000 = 0x800000.  The size of the flash memory is 8MiB or 0x800000 bytes.  Also, we may subtract the size of any partition (except the last) from the following offset and the result should equal the offset of that partition.  For instance, 0x280000 - 0x270000 = 0x10000, which is the offset of the factory partition.

To determine which partition-*.csv file your build is using, you must find which sdkconfig file is taking precedence.  To do this, first find the appropriate .cmake file in the board directory.  For instance, the micropython/ports/esp32/boards/GENERIC_S3 directory contains 3 .cmake files:
- mpconfigboard.cmake
- mpconfigvariant_FLASH_4M.cmake
- mpconfigvariant_SPIRAM_OCT.cmake

If you are using one of the variants, look in its .cmake file first, otherwise look in the mpconfigboard.cmake file.  We're looking for a line that points to an sdkconfig file **in the board directory**.  For instance, the mpconfigvariant_FLASH_4M.cmake file contains the following line:

```
boards/ESP32_GENERIC_S3/sdkconfig.flash_4m
```

In the sdkconfig.flash_4m file we find the following line:

```
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions-4MiB.csv"
```

So, we have found our partition file for the FLASH_4M variant.  However, the mpconfigvariant_SPIRAM_OCT.cmake file does not contain a line that points to an sdkconfig file **in the board directory**.  Instead, its partition table will be specified in the default mpconfigboard.cmake file, which contains the following line:

```
boards/ESP32_GENERIC_S3/sdkconfig.board
```

In the sdkconfig.board file, we find the following line:

```
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions-8MiB.csv"
```
So both the base GENERIC_S3 board and the SPIRAM_OCT variant use the partitions-8MiB.csv file.
