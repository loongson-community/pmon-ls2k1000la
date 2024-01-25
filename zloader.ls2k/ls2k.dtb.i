# 1 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
/dts-v1/;
/ {
 model = "loongson,LS2K1000_PAI_UDB_V1_5";

 compatible = "loongson,ls2k";
 #address-cells = <2>;
 #size-cells = <2>;

 aliases {
  ethernet0 = &gmac0;
  ethernet1 = &gmac1;
  serial0 = &cpu_uart0;
  serial1 = &cpu_uart3;
  serial2 = &cpu_uart4;
  serial3 = &cpu_uart5;
  i2c0 = &i2c0;
  i2c1 = &i2c1;
 };

 chosen {
  stdout-path = "serial0:115200n8";
  bootargs = "earlycon";
 };

 memory {
  name = "memory";
  device_type = "memory";
  reg = <0 0x00200000 0 0x0ee00000
   0 0x90000000 0 0x70000000>;
 };
# 54 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
 memalloc@0x90000000 {
  compatible = "loongson,ls-memalloc";
  reg = <0 0x90000000 0 0x20000000>;
 };

 cpus {
  #address-cells = <1>;
  #size-cells = <0>;

  cpu0: cpu@0 {
   device_type = "cpu";
   compatible = "loongarch";
   reg=<0>;
   numa-node-id = <0>;
  };

  cpu1: cpu@1 {
   device_type = "cpu";
   compatible = "loongarch";
   reg=<1>;
   numa-node-id = <0>;
  };
 };

 cpuic: interrupt-controller {
  compatible = "loongson,cpu-interrupt-controller";
  interrupt-controller;
  #interrupt-cells = <1>;
 };

 icu: interrupt-controller@1fe01400 {
  compatible = "loongson,2k1000-icu";
  interrupt-controller;
  #interrupt-cells = <1>;
  reg = <0 0x1fe01400 0 0x40
   0 0x1fe01040 0 16>;
  interrupt-parent = <&cpuic>;
  interrupt-names = "cascade";
  interrupts = <3>;
 };

 soc {
  compatible = "ls,nbus", "simple-bus";
  #address-cells = <2>;
  #size-cells = <2>;
  ranges = <0 0x10000000 0 0x10000000 0 0x10000000
   0 0x2000000 0 0x2000000 0 0x2000000
   0 0x20000000 0 0x20000000 0 0x10000000
   0 0x40000000 0 0x40000000 0 0x40000000
   0xfe 0x00000000 0xfe 0x00000000 0 0x40000000>;

  dma-coherent;

  scfg: scfg@1fe00000 {
   compatible = "loongson,2k1000la-scfg";
   reg = <0 0x1fe00000 0 0x3ffc>;
   little-endian;
  };


  pctrl:pinctrl@1fe00420 {
   compatible = "loongson,2k1000-pinctrl";
   reg = <0x1fe00420 0x18>;
# 134 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
   hda_default:hda {
    mux1 {
     groups ="hda";
     function ="hda";
    };

    mux2 {
     groups ="i2s";
     function ="gpio";
    };
   };
# 160 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
            nand_default:nand {
                mux {
                    groups ="nand";
                    function ="gpio";
                };
            };
  };

  cpu_uart0: serial@0x1fe20000 {
   compatible = "ns16550a";
   reg = <0 0x1fe20000 0 0x10>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <0>;
   no-loopback-test;
  };

  cpu_uart3: serial@0x1fe20300 {
   compatible = "ns16550a";
   reg = <0 0x1fe20300 0 0x10>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <0>;
   no-loopback-test;
  };

  cpu_uart4: serial@0x1fe20400 {
   compatible = "ns16550a";
   reg = <0 0x1fe20400 0 0x10>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <1>;
   no-loopback-test;
  };

  cpu_uart5: serial@0x1fe20500 {
   compatible = "ns16550a";
   reg = <0 0x1fe20500 0 0x10>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <1>;
   no-loopback-test;
  };

  pioA:gpio@0x1fe00500 {
   compatible = "ls,ls2k-gpio", "ls,ls-gpio";
   reg = <0 0x1fe00500 0 0x38>;
   ngpios = <64>;
   gpio-controller;
   #gpio-cells = <2>;

   interrupt-parent = <&icu>;
   interrupts =
    <60>, <61>, <62>, <63>, <58>, <58>,
    <58>, <58>, <58>, <58>, <58>, <58>,
    <58>, <58>, <58>, <>, <58>, <58>,
    <58>, <58>, <58>, <58>, <58>, <58>,
    <58>, <58>, <58>, <58>, <58>, <58>,
    <58>, <58>, <59>, <59>, <59>, <59>,
    <59>, <>, <59>, <59>, <59>, <59>,
    <>, <>, <59>, <59>, <59>, <59>,
    <59>, <59>, <59>, <59>, <59>, <59>,
    <59>, <59>, <59>, <59>, <59>, <59>,
    <59>, <59>, <59>, <59>;
  };

  pmc: syscon@0x1fe27000 {
   compatible = "syscon";
   reg = <0x0 0x1fe27000 0x0 0x58>;
  };

  reboot {
   compatible ="syscon-reboot";
   regmap = <&pmc>;
   offset = <0x30>;
   mask = <0x1>;
  };

  poweroff {
   compatible ="syscon-poweroff";
   regmap = <&pmc>;
   offset = <0x14>;
   mask = <0x3c00>;
   value = <0x3c00>;
  };

  otg@0x40000000 {



   compatible = "loongson,ls2k-otg", "loongson,dwc2";

   reg = <0 0x40000000 0 0x40000>;
   interrupt-parent = <&icu>;
   interrupts = <49>;
   dma-mask = <0x0 0xffffffff>;
            dr_mode = "otg";
  };

  ohci@0x40070000 {
   compatible = "loongson,ls2k-ohci", "generic-ohci";
   reg = <0 0x40070000 0 0x8000>;
   interrupt-parent = <&icu>;
   interrupts = <51>;
   dma-mask = <0x0 0xffffffff>;
  };

  ehci@0x40060000 {
   compatible = "loongson,ls2k-ehci", "generic-ehci";
   reg = <0 0x40060000 0 0x8000>;
   interrupt-parent = <&icu>;
   interrupts = <50>;

   dma-mask = <0 0xffffffff>;
  };

  spi0: spi@0x1fff0220{
   compatible = "loongson,ls-spi";
   #address-cells = <1>;
   #size-cells = <0>;
   reg = <0 0x1fff0220 0 0x10>;
   spidev@0{
    compatible = "jedec,spi-nor";
    spi-max-frequency = <40000000>;
    reg = <0>;
   };
   spidev@1{
    compatible = "rohm,dh2228fv";
    spi-max-frequency = <40000000>;
    reg = <1>;
   };
  };

  i2c0: i2c@1fe21000 {
   compatible = "loongson,ls-i2c";
   reg = <0 0x1fe21000 0 0x8>;
   interrupt-parent = <&icu>;
   interrupts = <22>;
   #address-cells = <1>;
   #size-cells = <0>;



   eeprom@57 {
    compatible = "atmel,24c16";
    reg = <0x50>;
    pagesize = <16>;
   };

   gt911@5d {
    compatible = "goodix,gt911";
    reg = <0x5d>;
    reset-gpios = <&pioA 2 0>;
    irq-gpios = <&pioA 1 0>;
    interrupt-parent = <&pioA>;
    interrupts = <1 8>;
    status = "okay";
   };
  };

  i2c1: i2c@1fe21800 {
   #address-cells = <1>;
   #size-cells = <0>;

   compatible = "loongson,ls-i2c";
   reg = <0 0x1fe21800 0 0x8>;
   interrupt-parent = <&icu>;
   interrupts = <23>;



   sil9022acnu@39 {
    compatible = "sil,sii9022-cripple";

    reg = <0x39>;

    hpd-gpios = <&pioA 0 0>;

    interrupt-parent = <&pioA>;

    status = "okay";

    ports {
     #address-cells = <1>;
     #size-cells = <0>;

     port@0 {
      reg = <0>;

      hdmi_encoder_in: endpoint {
       remote-endpoint = <&dvo1>;
      };
     };

     port@1 {
      reg = <1>;

      hdmi_encoder_out: endpoint {
       remote-endpoint = <&hdmi_connector_in>;
      };
     };
    };
   };

   codec@1a {
    compatible = "codec_uda1342";
    reg = <0x1a>;
   };
  };
# 402 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
  dc@0x400c0000 {
   compatible = "loongson,display-subsystem";
   reg = <0 0x400c0000 0 0x00010000>;
   interrupt-parent = <&icu>;
   interrupts = <28>;
   dma-mask = <0x00000000 0xffffffff>;

   #address-cells = <1>;
   #size-cells = <0>;

   ports {
    #address-cells = <1>;
    #size-cells = <0>;

    port@0 {
     reg = <0>;
     dvo0: endpoint {
      remote-endpoint = <&panel_in>;
     };
    };

    port@1 {
     reg = <1>;
     dvo1: endpoint {
      remote-endpoint = <&hdmi_encoder_in>;
     };
    };
   };
  };

  gpu@0x40080000 {
   compatible = "vivante,gc";
   reg = <0 0x40080000 0 0x00040000>;
   interrupt-parent = <&icu>;
   interrupts = <29>;
   dma-mask = <0x00000000 0xffffffff>;
  };

  ahci@0x400e0000 {
   compatible = "snps,spear-ahci";
   reg = <0 0x400e0000 0 0x10000>;
   interrupt-parent = <&icu>;
   interrupts = <19>;
   dma-mask = <0x0 0xffffffff>;
  };

  rtc0: rtc@1fe27800{
   #address-cells = <1>;
   #size-cells = <1>;
   compatible = "loongson,ls-rtc";
   reg = <0 0x1fe27800 0 0x100>;
   interrupt-parent = <&icu>;
   interrupts = <52>;
  };

  pwm0: pwm@1fe22000{
   compatible = "loongson,ls2k-pwm";
   reg = <0 0x1fe22000 0 0x10>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <24>;
   #pwm-cells = <2>;


  };

  pwm1: pwm@1fe22010{
   compatible = "loongson,ls2k-pwm";
   reg = <0 0x1fe22010 0 0x10>;
   clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <25>;
   #pwm-cells = <2>;


  };

  gmac0: ethernet@0x40040000 {
   compatible = "snps,dwmac-3.70a", "ls,ls-gmac";
   reg = <0 0x40040000 0 0x8000>;
   interrupt-parent = <&icu>;
   interrupts = <12 13>;
   interrupt-names = "macirq", "eth_wake_irq";
   mac-address = [ 64 48 48 48 48 60 ];
   phy-mode = "rgmii";
   bus_id = <0x0>;
   phy_addr = <0xffffffff>;
   dma-mask = <0xffffffff 0xffffffff>;
  };

  gmac1: ethernet@0x40050000 {
   compatible = "snps,dwmac-3.70a", "ls,ls-gmac";
   reg = <0 0x40050000 0 0x8000>;
   interrupt-parent = <&icu>;
   interrupts = <14 15>;
   interrupt-names = "macirq", "eth_wake_irq";
   mac-address = [ 64 48 48 48 48 61 ];
   phy-mode = "rgmii";
   bus_id = <0x1>;
   phy_addr = <0xffffffff>;
   dma-mask = <0xffffffff 0xffffffff>;
  };






  apbdma: apbdma@1fe00438{
   compatible = "loongson,ls-apbdma";
   reg = <0 0x1fe00438 0 0x8>;
   #config-nr = <2>;
  };





  dma0: dma@1fe00c00 {
   compatible = "loongson,ls-apbdma-0";
   reg = <0 0x1fe00c00 0 0x8>;
   apbdma-sel = <&apbdma 0x0 0x0>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };

  dma1: dma@1fe00c10 {
   compatible = "loongson,ls-apbdma-1";
   reg = <0 0x1fe00c10 0 0x8>;
   apbdma-sel = <&apbdma 0x5 0x1>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };

  dma2: dma@1fe00c20 {
   compatible = "loongson,ls-apbdma-2";
   reg = <0 0x1fe00c20 0 0x8>;
   apbdma-sel = <&apbdma 0x6 0x2>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };

  dma3: dma@1fe00c30 {
   compatible = "loongson,ls-apbdma-3";
   reg = <0 0x1fe00c30 0 0x8>;
   apbdma-sel = <&apbdma 0x7 0x3>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };

  dma4: dma@1fe00c40 {
   compatible = "loongson,ls-apbdma-4";
   apbdma-sel = <&apbdma 0x0 0x0>;
   reg = <0 0x1fe00c40 0 0x8>;
   #dma-cells = <1>;
   dma-channels = <1>;
   dma-requests = <1>;
  };
# 582 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
  nand@0x1fe26040{
   #address-cells = <2>;
   compatible = "loongson,ls-nand";
   reg = <0 0x1fe26040 0 0x0
          0 0x1fe26000 0 0x20>;
   interrupt-parent = <&icu>;
   interrupts = <44>;
   interrupt-names = "nand_irq";
   pinctrl-0 = <&nand_default>;
   pinctrl-names = "default";

   dmas = <&dma0 1>;
   dma-names = "nand_rw";
   dma-mask = <0xffffffff 0xffffffff>;

   number-of-parts = <0x2>;
   partitions {
    compatible = "fixed-partitions";
    #address-cells = <1>;
    #size-cells = <1>;

    partition@0 {
     label = "kernel_partition";
     reg = <0 0x0000000 0 0x01400000>;
    };

    partition@0x01400000 {
     label = "os_partition";
     reg = <0 0x01400000 0 0x0>;
    };
   };
  };





  can0: can@1fe20c00{
   compatible = "nxp,sja1000";
   reg = <0 0x1fe20c00 0 0xff>;
            nxp,tx-output-config = <0x16>;
   nxp,external-clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <16>;
  };
  can1: can@1fe20d00{
   compatible = "nxp,sja1000";
   reg = <0 0x1fe20d00 0 0xff>;
            nxp,tx-output-config = <0x16>;
   nxp,external-clock-frequency = <125000000>;
   interrupt-parent = <&icu>;
   interrupts = <17>;
  };
  hda@0x400d0000 {
   compatible = "loongson,ls2k-audio";
   reg = <0 0x400d0000 0 0xffff>;
   interrupt-parent = <&icu>;
   interrupts = <4>;
   pinctrl-0 = <&hda_default>;
   pinctrl-names = "default";
  };

  vpu@0x79000000 {
   compatible = "loongson,ls-vpu";
   reg = <0 0x79000000 0 0xffff>;
   interrupt-parent = <&icu>;
   interrupts = <30>;
  };
# 660 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
  pcie@0 {
   compatible = "loongson,ls2k1000-pci";
   #interrupt-cells = <1>;
   bus-range = <0x1 0x16>;
   #size-cells = <2>;
   #address-cells = <3>;

   reg = < 0xfe 0x00000000 0 0x20000000>;
   ranges = <0x2000000 0x0 0x60000000 0 0x60000000 0x0 0x20000000
    0x01000000 0 0x00008000 0 0x18008000 0x0 0x8000>;

   pcie0_port0: pci_bridge@9,0 {
    compatible = "pciclass060400",
         "pciclass0604";
    reg = <0x4800 0x0 0x0 0x0 0x0>;
    interrupts = <32>;
    interrupt-parent = <&icu>;

    #interrupt-cells = <1>;
    interrupt-map-mask = <0 0 0 0>;
    interrupt-map = <0 0 0 0 &icu 32>;
   };

   pcie0_port1: pci_bridge@10,0 {
    compatible = "pciclass060400",
         "pciclass0604";
    reg = <0x5000 0x0 0x0 0x0 0x0>;
    interrupts = <33>;
    interrupt-parent = <&icu>;

    #interrupt-cells = <1>;
    interrupt-map-mask = <0 0 0 0>;
    interrupt-map = <0 0 0 0 &icu 33>;
   };

   pcie0_port2: pci_bridge@11,0 {
    compatible = "pciclass060400",
         "pciclass0604";
    reg = <0x5800 0x0 0x0 0x0 0x0>;
    interrupts = <34>;
    interrupt-parent = <&icu>;

    #interrupt-cells = <1>;
    interrupt-map-mask = <0 0 0 0>;
    interrupt-map = <0 0 0 0 &icu 34>;
   };

   pcie_port3: pci_bridge@12,0 {
    compatible = "pciclass060400",
         "pciclass0604";
    reg = <0x6000 0x0 0x0 0x0 0x0>;
    interrupts = <35>;
    interrupt-parent = <&icu>;

    #interrupt-cells = <1>;
    interrupt-map-mask = <0 0 0 0>;
    interrupt-map = <0 0 0 0 &icu 35>;
   };

   pcie1_port0: pci_bridge@13,0 {
    compatible = "pciclass060400",
         "pciclass0604";
    reg = <0x6800 0x0 0x0 0x0 0x0>;
    interrupts = <36>;
    interrupt-parent = <&icu>;

    #interrupt-cells = <1>;
    interrupt-map-mask = <0 0 0 0>;
    interrupt-map = <0 0 0 0 &icu 36>;
   };

   pcie1_port1: pci_bridge@14,0 {
    compatible = "pciclass060400",
         "pciclass0604";
    reg = <0x7000 0x0 0x0 0x0 0x0>;
    interrupts = <37>;
    interrupt-parent = <&icu>;

    #interrupt-cells = <1>;
    interrupt-map-mask = <0 0 0 0>;
    interrupt-map = <0 0 0 0 &icu 37>;
   };

  };
 };

 lcd_backlight: backlight {
  compatible = "loongson,lsdc-pwm", "pwm-backlight";
  pwms = <&pwm0 0 4000000 0>;
  brightness-levels = <
    99 98 97 96 95 94 93 92
    91 90 89 88 87 86 85 84
    83 82 81 80 79 78 77 76
    75 74 73 72 71 70 69 68
    67 66 65 64 63 62 61 60
    59 58 57 56 55 54 53 52
    51 50 49 48 47 46 45 44
    43 42 41 40 39 38 37 36
    35 34 33 32 31 30 29 28
    27 26 25 24 23 22 21 20
    19 18 17 16 15 14 13 12
    11 10 9 8 7 6 5 4
     3 2 1 0 >;
  default-brightness-level = <80>;
 };

 regulators {
  compatible = "simple-bus";
  #address-cells = <1>;
  #size-cells = <0>;

  vdd_5v0_sys: regulator@0 {
   compatible = "regulator-fixed";
   reg = <0>;
   regulator-name = "VDD_5V0_SYS";
   regulator-min-microvolt = <5000000>;
   regulator-max-microvolt = <5000000>;
   regulator-always-on;
   regulator-boot-on;
  };

  vdd_lcd: regulator@3 {
   compatible = "regulator-fixed";
   reg = <3>;
   regulator-name = "+VDD_LED";
   regulator-min-microvolt = <5000000>;
   regulator-max-microvolt = <5000000>;
   regulator-always-on;
   regulator-boot-on;
   enable-active-high;
   gpio = <&pioA 3 0>;
   vin-supply = <&vdd_5v0_sys>;
  };
 };

 panel: display@0 {


  compatible = "forlinx,LCD070CG_1024X600_DC21", "avic,tm070ddh03";





  backlight = <&lcd_backlight>;
# 813 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
  power-supply = <&vdd_lcd>;


  status = "okay";

  #address-cells = <1>;
  #size-cells = <0>;

  port@0 {
   reg = <0>;

   #address-cells = <1>;
   #size-cells = <0>;

   panel_in: endpoint@0 {
    reg = <0>;
    remote-endpoint = <&dvo0>;
   };
  };
 };

 hdmi-connector@1 {
  compatible = "hdmi-connector";
  type = "hdmi-a";

  status = "okay";

  port@0 {
   hdmi_connector_in: endpoint {
    remote-endpoint = <&hdmi_encoder_out>;
   };
  };
 };
# 855 "/home/ddh/ls2k1000la/PMON/pmon-ls2k1000la_v0.33_ts/pmon-ls2k1000la/zloader.ls2k/../Targets/ls2k/conf/ls2k_lspi.dts"
};
