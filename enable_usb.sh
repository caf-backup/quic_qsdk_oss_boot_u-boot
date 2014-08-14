#!/bin/sh
# Script to apply USB patches
wget -O include/linux/usb/ch9.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/linux/usb/ch9.h;h=d1d732c2838d3a48788c16fe87bd9552a8558ab8;hb=82651c39f6544e932fb86853bf9a648414ccca9a' &&
wget -O common/usb_storage.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=common/usb_storage.c;h=457970f770ce774b6c3e5f461f3216b411258951;hb=605bd75af565011aa46e6d80a32e2aa03aff8159' &&
wget -O common/cmd_usb.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=common/cmd_usb.c;h=70e803b5bb6c6dfe27cac2ef8ebed8cc10f3b075;hb=6497c66704d03956e7ea49b54fcaa38740736416' &&
wget -O common/usb_hub.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=common/usb_hub.c;h=0d79ec3ea84abfc8d61988cbfc30c8ba17cec60e;hb=55f4b57542de9f4bee8dc0b7ca70686bd20e2aa4' &&
wget -O include/usb_defs.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/usb_defs.h;h=236a5ecdf6f9fda347aafe2bded83e42fbdab372;hb=e3d7440c2212497b33f0b12f5112f1a99e3f201c' &&
wget -O drivers/usb/host/xhci-mem.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=drivers/usb/host/xhci-mem.c;h=89908e8a8062c5fc3348dfdd4ac81f25d2c3ca04;hb=5853e1335cfc76c230cc28c4226b4d513ddb7542' &&
wget -O drivers/usb/host/xhci-ring.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=drivers/usb/host/xhci-ring.c;h=19c3ec62118d95c77957679de5cca93368ab24a3;hb=5853e1335cfc76c230cc28c4226b4d513ddb7542' &&
wget -O drivers/usb/host/xhci.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=drivers/usb/host/xhci.c;h=57fa4ff9a77cfbe92540cb4cf4671acad35010a6;hb=5853e1335cfc76c230cc28c4226b4d513ddb7542' &&
wget -O drivers/usb/host/xhci.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=drivers/usb/host/xhci.h;h=ceb1573d86b043c8d3c507a8eed55ca484bb14de;hb=5853e1335cfc76c230cc28c4226b4d513ddb7542' &&
wget -O include/linux/usb/dwc3.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/linux/usb/dwc3.h;h=97d179a6e9e747ab657c4025796bfeb047f3a0bb;hb=13194f3b5f51d104bdfdd7ff5a7556136b6dc35c' &&
wget -O common/usb.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=common/usb.c;h=414f9c27fb88664e2731d581f9d3fdfea7029ebc;hb=bba679144d25b91bcd7befff5a96728a30875f54' &&
wget -O include/usb.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/usb.h;h=b4fea52cbffb4b2a91f94b63d329f08363b311a8;hb=bba679144d25b91bcd7befff5a96728a30875f54' &&
wget -O include/ide.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/ide.h;h=158e1beaf9510385353dde29dfbf33c7665125d5;hb=24a3fdd64d449c1acd4ccc3636f8bf02e903b6f8' &&
wget -O include/part.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/part.h;h=f7c7cc59fca2dccb41635e1f28666aed60234449;hb=0472fbfd3250d1a33d3de78afdcbf24f78ac026b' &&
wget -O disk/part.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=disk/part.c;h=0dd4b0f0c63a2a0ce118ffc21de383b5fd1f325e;hb=10a37fd7a40826c43a63591855346adf1a1ac02d' &&
wget -O common/Makefile 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=common/Makefile;h=482795ed142dae323b2e294855757359a69a2fb6;hb=7405a133101e009c760b98dd4dbcdc49b82ec3b3' &&
wget -O common/cmd_disk.c 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=common/cmd_disk.c;h=e73fc4e45692d511dff3c0ecc172dea71b07d415;hb=7405a133101e009c760b98dd4dbcdc49b82ec3b3' &&
wget -O include/command.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/command.h;h=1f06aa1819023c0869b5c30ddeff4fe949becf19;hb=7405a133101e009c760b98dd4dbcdc49b82ec3b3' &&
wget -O include/linux/types.h 'http://git.denx.de/?p=u-boot.git;a=blob_plain;f=include/linux/types.h;h=f07ba41b72cc632999760643afdf79e74f1eef41;hb=472d546054dadacca91530bad42ad06f6408124e' &&
sed -i '/^#ifdef CONFIG_IPQ806X_USB/i#define CONFIG_IPQ806X_USB\n'  include/configs/ipq806x_cdp.h
