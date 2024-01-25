/** @file
  Sample ACPI Platform Driver

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  Copyright (c) 2018 Loongson Technology Corporation Limited (www.loongson.cn).
  All intellectual property rights(Copyright, Patent and Trademark) reserved.

  Any violations of copyright or other intellectual property rights of the Loongson
  Technology Corporation Limited will be held accountable in accordance with the law,
  if you (or any of your subsidiaries, corporate affiliates or agents) initiate directly
  or indirectly any Intellectual Property Assertion or Intellectual Property Litigation:
  (i) against Loongson Technology Corporation Limited or any of its subsidiaries or corporate affiliates,
  (ii) against any party if such Intellectual Property Assertion or Intellectual Property Litigation arises
  in whole or in part from any software, technology, product or service of Loongson Technology Corporation Limited
  or any of its subsidiaries or corporate affiliates, or (iii) against any party relating to the Software.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION).

**/

Scope (\_SB)
{
  Device (GPO0)
  {
    Name (_HID, "LOON0002")  // _HID: Hardware ID
    Name (_ADR, Zero)  // _ADR: Address
    Name (_UID, One)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x00000000100E0000, // Range Minimum
        0x00000000100E0BFF, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000C00, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
      {
       0x0000007C,
       0x0000007D,
       0x0000007E,
       0x0000007F,
       0x0000007B,
      }
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
      Package (0x06)
      {
        Package (0x02)
        {
          "conf_offset",
          0x800
        },

        Package (0x02)
        {
          "out_offset",
          0x900
        },

        Package (0x02)
        {
          "in_offset",
          0xa00
        },

        Package (0x02)
        {
          "gpio_base",
          16
        },

        Package (0x02)
        {
          "ngpios",
          57
        },

        Package (0x02)
        {
          "gsi_idx_map",
          Package (0x39)
          {
            0x0000,
            0x0001,
            0x0002,
            0x0003,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004,
            0x0004
          }
        }
      }
    })
  }

  Device (COMA)
  {
    Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
    Name (_UID, 0x0)  // _UID: Unique ID
    Name (_CCA, One)  // _CCA: Cache Coherency Attribute
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x000000001fe001e0, // Range Minimum
        0x000000001fe001e7, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000008, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        26,
      }
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
      Package (0x01)
      {
        Package (0x02)
        {
          "clock-frequency",
          100000000
        }
      }
    })
  }
  Device (COMB)
  {
    Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
    Name (_UID, 0x1)  // _UID: Unique ID
    Name (_CCA, One)  // _CCA: Cache Coherency Attribute
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010080000, // Range Minimum
        0x00000000100800FF, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000100, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        72,
      }
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
      Package (0x01)
      {
        Package (0x02)
        {
          "clock-frequency",
          50000000
        }
      }
    })
  }
  Device (COMC)
  {
    Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
    Name (_UID, 0x2)  // _UID: Unique ID
    Name (_CCA, One)  // _CCA: Cache Coherency Attribute
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010080100, // Range Minimum
        0x00000000100801FF, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000100, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        72,
      }
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
      Package (0x01)
      {
        Package (0x02)
        {
          "clock-frequency",
          50000000
        }
      }
    })
  }
  Device (COMD)
  {
    Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
    Name (_UID, 0x3)  // _UID: Unique ID
    Name (_CCA, One)  // _CCA: Cache Coherency Attribute
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010080200, // Range Minimum
        0x00000000100802FF, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000100, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        72,
      }
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
      Package (0x01)
      {
        Package (0x02)
        {
          "clock-frequency",
          50000000
        }
      }
    })
  }
  Device (COME)
  {
    Name (_HID, "PNP0501" /* 16550A-compatible COM Serial Port */)  // _HID: Hardware ID
    Name (_UID, 0x4)  // _UID: Unique ID
    Name (_CCA, One)  // _CCA: Cache Coherency Attribute
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010080300, // Range Minimum
        0x00000000100803FF, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000100, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        72,
      }
    })
    Name (_DSD, Package (0x02)  // _DSD: Device-Specific Data
    {
      ToUUID ("daffd814-6eba-4d8c-8a91-bc9bbf4aa301") /* Device Properties for _DSD */,
      Package (0x01)
      {
        Package (0x02)
        {
          "clock-frequency",
          50000000
        }
      }
    })
  }
  Device (RTC)
  {
    Name (_HID, "LOON0001" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x00000000100d0100, // Range Minimum
        0x00000000100d01FF, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000100, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, )
      {
        116,
      }
    })
  }
  Device (I2C0)
  {
    Name (_HID, "LOON0004" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_UID, 0x0)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010090000, // Range Minimum
        0x0000000010090007, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000008, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        73,
      }
    })
  }
  Device (I2C1)
  {
    Name (_HID, "LOON0004" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_UID, 0x1)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010090100, // Range Minimum
        0x0000000010090107, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000008, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        73,
      }
    })
  }
  Device (I2C2)
  {
    Name (_HID, "LOON0004" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_UID, 0x2)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010090200, // Range Minimum
        0x0000000010090207, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000008, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        73,
      }
    })
  }
  Device (I2C3)
  {
    Name (_HID, "LOON0004" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_UID, 0x3)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010090300, // Range Minimum
        0x0000000010090307, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000008, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        73,
      }
    })
  }
  Device (I2C4)
  {
    Name (_HID, "LOON0004" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_UID, 0x4)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x0000000010090400, // Range Minimum
        0x0000000010090407, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000008, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        73,
      }
    })
  }
  Device (I2C5)
  {
    Name (_HID, "LOON0004" /* AT Real-Time Clock */)  // _HID: Hardware ID
    Name (_UID, 0x5)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
          0x0000000000000000, // Granularity
          0x0000000010090500, // Range Minimum
          0x0000000010090507, // Range Maximum
          0x0000000000000000, // Translation Offset
          0x0000000000000008, // Length
          ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        73,
      }
    })
  }

  Device (PWM0)
  {
    Name (_HID, "LOON0006") // _HID: Hardware ID
    Name (_UID, 0x0)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x00000000100a0000, // Range Minimum
        0x00000000100a000F, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000010, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        88,
      }
    })
  }
  Device (PWM1)
  {
    Name (_HID, "LOON0006") // _HID: Hardware ID
    Name (_UID, 0x1)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x00000000100a0100, // Range Minimum
        0x00000000100a010F, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000010, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        89,
      }
    })
  }
  Device (PWM2)
  {
    Name (_HID, "LOON0006") // _HID: Hardware ID
    Name (_UID, 0x2)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x00000000100a0200, // Range Minimum
        0x00000000100a020F, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000010, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        90,
      }
    })
  }
  Device (PWM3)
  {
    Name (_HID, "LOON0006") // _HID: Hardware ID
    Name (_UID, 0x3)  // _UID: Unique ID
    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      QWordMemory (ResourceConsumer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        0x0000000000000000, // Granularity
        0x00000000100a0300, // Range Minimum
        0x00000000100a030F, // Range Maximum
        0x0000000000000000, // Translation Offset
        0x0000000000000010, // Length
        ,, , AddressRangeMemory, TypeStatic)
      Interrupt (ResourceConsumer, Level, ActiveHigh, Shared, ,, )
      {
        91,
      }
    })
  }
  Device (NIPM)
  {
    Name (_HID, "IPI0001")  // _HID: Hardware ID
    Name(_STR, Unicode("IPMI_KCS"))
    Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
    {
      Return (ResourceTemplate ()
      {
        IO (Decode16,
        0x0CA2,             // Range Minimum
        0x0CA2,             // Range Maximum
        0x00,               // Alignment
        0x01,               // Length
        )
        IO (Decode16,
        0x0CA3,             // Range Minimum
        0x0CA3,             // Range Maximum
        0x00,               // Alignment
        0x01,               // Length
        )
      })
    }

    Method (_IFT, 0, NotSerialized)  // _IFT: IPMI Interface Type
    {
      Return (0x01)
    }

    Method (_SRV, 0, NotSerialized)  // _SRV: IPMI Spec Revision
    {
      Return (0x0200)
    }
  }
}
