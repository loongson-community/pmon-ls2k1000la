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
  Device (C000)
  {
    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
    Name (_UID, 1)  // _UID: Unique ID
    Name (_PXM, Zero)  // _PXM: Device Proximity
    Name (_STA, 0x0F)  // _STA: Status
    Name (_CPC, Package (0x15)  // _CPC: Continuous Performance Control
    {
      0x15,
      0x02,
      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000004, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000008, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000000C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000010, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000014, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000018, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000001C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000020, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000024, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000028, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000002C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000030, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000003C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000040, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      }
    })
    Name (_PSD, Package (0x01)  // _PSD: Power State Dependencies
    {
      Package (0x05)
      {
          0x05,
          Zero,
          0x00,
          0xFD,
          0x4
      }
    })
  }

  Device (C001)
  {
    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
    Name (_UID, 2)  // _UID: Unique ID
    Name (_PXM, Zero)  // _PXM: Device Proximity
    Name (_STA, 0x0F)  // _STA: Status
    Name (_CPC, Package (0x15)  // _CPC: Continuous Performance Control
    {
      0x15,
      0x02,
      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000004, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000008, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000000C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000010, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000014, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000018, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000001C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000020, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000024, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000028, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000002C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000030, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000003C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000040, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      }
    })
    Name (_PSD, Package (0x01)  // _PSD: Power State Dependencies
    {
      Package (0x05)
      {
          0x05,
          Zero,
          0x00,
          0xFD,
          0x4
      }
    })
  }

  Device (C002)
  {
    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
    Name (_UID, 3)  // _UID: Unique ID
    Name (_PXM, Zero)  // _PXM: Device Proximity
    Name (_STA, 0x0F)  // _STA: Status
    Name (_CPC, Package (0x15)  // _CPC: Continuous Performance Control
    {
      0x15,
      0x02,
      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000004, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000008, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000000C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000010, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000014, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000018, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000001C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000020, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000024, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000028, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000002C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000030, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000003C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000040, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      }
    })
    Name (_PSD, Package (0x01)  // _PSD: Power State Dependencies
    {
        Package (0x05)
        {
            0x05,
            Zero,
            0x00,
            0xFD,
            0x4
        }
    })
  }

  Device (C003)
  {
    Name (_HID, "ACPI0007" /* Processor Device */)  // _HID: Hardware ID
    Name (_UID, 4)  // _UID: Unique ID
    Name (_PXM, Zero)  // _PXM: Device Proximity
    Name (_STA, 0x0F)  // _STA: Status
    Name (_CPC, Package (0x15)  // _CPC: Continuous Performance Control
    {
      0x15,
      0x02,
      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000004, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000008, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000000C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000010, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000014, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000018, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000001C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000020, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000024, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000028, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000002C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000030, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x000000000000003C, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (PCC,
              0x20,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000040, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      },

      ResourceTemplate ()
      {
          Register (SystemMemory,
              0x00,               // Bit Width
              0x00,               // Bit Offset
              0x0000000000000000, // Address
              ,)
      }
    })
    Name (_PSD, Package (0x01)  // _PSD: Power State Dependencies
    {
      Package (0x05)
      {
          0x05,
          Zero,
          0x00,
          0xFD,
          0x4
      }
    })
  }
}
