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

// NOTE: Don't delete PCI1, it's a template!

Scope (\_SB)
{
  Device (PCI0)
  {
    Name (_BBN, 0x00)      // _BBN: BIOS Bus Number
    Name (_ADR, 0x00)      // _ADR: Address
    Name (_SEG, 0x00)      // _SEG: Segment

    Name (_HID, EisaId ("PNP0A08") /* PCI Express Bus */)   // _HID: Hardware ID
    Name (_CID, EisaId ("PNP0A03") /* PCI Bus */)       // _CID: Compatible ID
    Name (_CRS, ResourceTemplate ()             // _CRS: Current Resource Settings
    {
      WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode,
        0x0000,       // Granularity
        0x0000,       // Range Minimum
        0x00FF,       // Range Maximum
        0x0000,       // Translation Offset
        0x0100,       // Length
        ,,)
      WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
        0x0000,       // Granularity
        0x4000,       // Range Minimum /* io base need add 0xefdfcxxxxxx,0 ~ 0x4000 reserve for LPC */
        0x9FFF,       // Range Maximum
        0x0000,       // Translation Offset
        0x6000,       // Length
        ,,,)
      DWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
        0x00000000,     // Granularity
        0x40000000,     // Range Minimum
        0x7FFFFFFF,     // Range Maximum
        0x00000000,     // Translation Offset
        0x40000000,     // Length
        ,,,,)
      QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, Cacheable, ReadWrite,
        0x00000000,     // Granularity
        0x0E0080000000, // Range Minimum
        0x0EEFCFFFFFFF, // Range Maximum
        0x00000000,     // Translation Offset
        0xEF50000000,   // Length
        ,, QWDM, AddressRangeMemory, TypeStatic)
    })
    /* PCI interrupt route table */
    Name (_PRT, Package (0x3F)  // _PRT: PCI Routing Table
    {
      Package (0x04)
      {
        0x0003FFFF,
        Zero,
        Zero,
        0x4C
      },

      Package (0x04)
      {
        0x0003FFFF,
        One,
        Zero,
        0x4E
      },

      Package (0x04)
      {
        0x0004FFFF,
        Zero,
        Zero,
        0x71
      },

      Package (0x04)
      {
        0x0004FFFF,
        One,
        Zero,
        0x70
      },

      Package (0x04)
      {
        0x0005FFFF,
        Zero,
        Zero,
        0x73
      },

      Package (0x04)
      {
        0x0005FFFF,
        One,
        Zero,
        0x72
      },

      Package (0x04)
      {
        0x0006FFFF,
        Zero,
        Zero,
        0x5D
      },

      Package (0x04)
      {
        0x0006FFFF,
        One,
        Zero,
        0x5C
      },

      Package (0x04)
      {
        0x0006FFFF,
        0x02,
        Zero,
        0x57
      },

      Package (0x04)
      {
        0x0007FFFF,
        Zero,
        Zero,
        0x7A
      },

      Package (0x04)
      {
        0x0008FFFF,
        Zero,
        Zero,
        0x50
      },

      Package (0x04)
      {
        0x0008FFFF,
        One,
        Zero,
        0x51
      },

      Package (0x04)
      {
        0x0008FFFF,
        0x02,
        Zero,
        0x52
      },

      Package (0x04)
      {
        0x0009FFFF,
        Zero,
        Zero,
        0x60
      },

      Package (0x04)
      {
        0x0009FFFF,
        1,
        Zero,
        0x60
      },

      Package (0x04)
      {
        0x0009FFFF,
        2,
        Zero,
        0x60
      },

      Package (0x04)
      {
        0x0009FFFF,
        3,
        Zero,
        0x60
      },

      Package (0x04)
      {
        0x000AFFFF,
        Zero,
        Zero,
        0x61
      },

      Package (0x04)
      {
        0x000AFFFF,
        1,
        Zero,
        0x61
      },

      Package (0x04)
      {
        0x000AFFFF,
        2,
        Zero,
        0x61
      },

      Package (0x04)
      {
        0x000AFFFF,
        3,
        Zero,
        0x61
      },

      Package (0x04)
      {
        0x000BFFFF,
        Zero,
        Zero,
        0x62
      },

      Package (0x04)
      {
        0x000BFFFF,
        1,
        Zero,
        0x62
      },

      Package (0x04)
      {
        0x000BFFFF,
        2,
        Zero,
        0x62
      },

      Package (0x04)
      {
        0x000BFFFF,
        3,
        Zero,
        0x62
      },

      Package (0x04)
      {
        0x000CFFFF,
        Zero,
        Zero,
        0x63
      },

      Package (0x04)
      {
        0x000CFFFF,
        1,
        Zero,
        0x63
      },

      Package (0x04)
      {
        0x000CFFFF,
        2,
        Zero,
        0x63
      },

      Package (0x04)
      {
        0x000CFFFF,
        3,
        Zero,
        0x63
      },

      Package (0x04)
      {
        0x000DFFFF,
        Zero,
        Zero,
        0x64
      },

      Package (0x04)
      {
        0x000DFFFF,
        1,
        Zero,
        0x64
      },

      Package (0x04)
      {
        0x000DFFFF,
        2,
        Zero,
        0x64
      },

      Package (0x04)
      {
        0x000DFFFF,
        3,
        Zero,
        0x64
      },

      Package (0x04)
      {
        0x000EFFFF,
        Zero,
        Zero,
        0x65
      },

      Package (0x04)
      {
        0x000EFFFF,
        1,
        Zero,
        0x65
      },

      Package (0x04)
      {
        0x000EFFFF,
        2,
        Zero,
        0x65
      },

      Package (0x04)
      {
        0x000EFFFF,
        3,
        Zero,
        0x65
      },

      Package (0x04)
      {
        0x000FFFFF,
        Zero,
        Zero,
        0x68
      },

      Package (0x04)
      {
        0x000FFFFF,
        1,
        Zero,
        0x68
      },

      Package (0x04)
      {
        0x000FFFFF,
        2,
        Zero,
        0x68
      },

      Package (0x04)
      {
        0x000FFFFF,
        3,
        Zero,
        0x68
      },

      Package (0x04)
      {
        0x0010FFFF,
        Zero,
        Zero,
        0x69
      },

      Package (0x04)
      {
        0x0010FFFF,
        1,
        Zero,
        0x69
      },

      Package (0x04)
      {
        0x0010FFFF,
        2,
        Zero,
        0x69
      },

      Package (0x04)
      {
        0x0010FFFF,
        3,
        Zero,
        0x69
      },

      Package (0x04)
      {
        0x0011FFFF,
        Zero,
        Zero,
        0x6A
      },

      Package (0x04)
      {
        0x0011FFFF,
        1,
        Zero,
        0x6A
      },

      Package (0x04)
      {
        0x0011FFFF,
        2,
        Zero,
        0x6A
      },

      Package (0x04)
      {
        0x0011FFFF,
        3,
        Zero,
        0x6A
      },

      Package (0x04)
      {
        0x0012FFFF,
        Zero,
        Zero,
        0x6B
      },

      Package (0x04)
      {
        0x0012FFFF,
        1,
        Zero,
        0x6B
      },

      Package (0x04)
      {
        0x0012FFFF,
        2,
        Zero,
        0x6B
      },

      Package (0x04)
      {
        0x0012FFFF,
        3,
        Zero,
        0x6B
      },

      Package (0x04)
      {
        0x0013FFFF,
        Zero,
        Zero,
        0x66
      },

      Package (0x04)
      {
        0x0013FFFF,
        1,
        Zero,
        0x66
      },

      Package (0x04)
      {
        0x0013FFFF,
        2,
        Zero,
        0x66
      },

      Package (0x04)
      {
        0x0013FFFF,
        3,
        Zero,
        0x66
      },

      Package (0x04)
      {
        0x0014FFFF,
        Zero,
        Zero,
        0x67
      },

      Package (0x04)
      {
        0x0014FFFF,
        1,
        Zero,
        0x67
      },

      Package (0x04)
      {
        0x0014FFFF,
        2,
        Zero,
        0x67
      },

      Package (0x04)
      {
        0x0014FFFF,
        3,
        Zero,
        0x67
      },

      Package (0x04)
      {
        0x0017FFFF,
        Zero,
        Zero,
        0x53

      },

      Package (0x04)
      {
        0x0019FFFF,
        Zero,
        Zero,
        0x56
      }
    })
  }
}
