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

Scope (\_SB.PCI0.LPC)
{
  Device (EC)
  {
    Name (_HID, EISAID("PNP0C09") /* Embedded Controller Device */)  // _HID: Hardware ID
    Method(_STA,0)
    {
      Return(0x1f)       // Hide device
    }
    Name (_UID, 0x0)  // _UID: Unique ID
    Name (_GPE, 0x4)  // _GPE: General Purpose Events
    //Method (_REG, 2, NotSerialized)  // _REG: Region Availability

    Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
    {
      IO (Decode16,
        0x0062,             // Range Minimum
        0x0062,             // Range Maximum
        0x01,               // Alignment
        0x01,               // Length
      )
      IO (Decode16,
        0x0066,             // Range Minimum
        0x0066,             // Range Maximum
        0x01,               // Alignment
        0x01,               // Length
      )
    })

    OperationRegion (ECOR, EmbeddedControl, 0x00, 0x0100)
    Field (ECOR, ByteAcc, NoLock, Preserve)
    {
      Offset  (0x1E),   /*Read current temperature.  */
      CTMP,   8,
      Offset (0x5A),
      BLAC,   8,
      Offset  (0x80),   /*Read current power status.  */
      AST1,   1,         // Bat1 Attach/Detach. 1=Attached
      BCF2,   1,         // Bat2 charge full
      BCG2,   1,         // Bat2 is Being charge
      BCF1,   1,         // Bat1 charge full
      BCG1,   1,        // Bat1 is Being charge
      LSTE,   1,        // Lid State (Lid Open = 1)
      AST2,   1,        // Bat2 Attach/detach 1=Attached
      RPWR,   1,        // Real AC Power (AC Present = 1)
      Offset  (0x92),   /* Battery Voltage Low byte. */
      BVL,    8,
      Offset  (0x93),   /* Battery Voltage High byte. */
      BVH,    8,
      Offset  (0x94),   /* Battery Current Low byte. */
      BCL,    8,
      Offset  (0x95),   /* Battery Current High byte. */
      BCH,    8,
      Offset  (0x98),   /* Battery RemainingCapacity Low byte */
      BRCL,   8,
      Offset  (0x99),   /* Battery RemainingCapacity High byte */
      BRCH,   8,
      Offset  (0x9A),   /*Battery FullChargeCapacity Low byte */
      BFCL,   8,
      Offset  (0x9B),   /*Battery FullChargeCapacity High byte */
      BFCH,   8,
      Offset  (0xA2),   /*Battery DesignVoltage Low byte. */
      BDVL,   8,
      Offset  (0xA3),   /*Battery DesignVoltage High byte.  */
      BDVH,   8,
      Offset  (0xAE),   /*Battery DesignCapacity Low byte. */
      BDCL,   8,
      Offset  (0xAF),   /*Battery DesignCapacity High byte. */
      BDCH,   8,
      Offset  (0xB0),   /*Battery SerialNumber Low byte.  */
      BSNL,   8,
      Offset  (0xB1),   /*Battery SerialNumber High byte.  */
      BSNH,   8
    }

    Method (_Q10, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1001)
    }

    Name (ELID, Zero)
    Method (_Q40, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1001)
    }

    Method (_Q3A, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1002)
    }

    Method (_Q36, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1003)
    }

    Method (_Q37, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1004)
    }

    Method (_Q38, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1005)
    }

    Method (_Q39, 0, NotSerialized)  // _Qxx: EC Query
    {
      Notify (HKEY, 0x1006)
    }

    Method (_Q21, 0, NotSerialized)  // _Qxx: EC Query
    {
      Store (One, ELID) /* \_SB_.PCI0.LPC_.EC__.ELID */
      Notify (HKEY, 0x2007)
    }

    PowerResource (PUBS, 0x03, 0x0000)
    {
      Method (_STA, 0, NotSerialized)  // _STA: Status
      {
        Return (One)
      }

      Method (_ON, 0, NotSerialized)  // _ON_: Power On
      {
      }

      Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
      {
      }
    }

    Device (HKEY)
    {
      Name (_HID, "LOON0000")  // _HID: Hardware ID
      Name (KMAP, Package (0x07)
      {
        Package (0x03)
        {
          One,
          One,
          0x8E
        },

        Package (0x03)
        {
          One,
          0x02,
          0xE3
        },

        Package (0x03)
        {
          One,
          0x03,
          0x0212
        },

        Package (0x03)
        {
          One,
          0x04,
          0x01AF
        },

        Package (0x03)
        {
          One,
          0x05,
          0xE0
        },

        Package (0x03)
        {
          One,
          0x06,
          0xE1
        },

        Package (0x03)
        {
          0x02,
          0x07,
          Zero
        }
      })
      Method (ECBS, 1, Serialized)
      {
        Store (Arg0, BLAC)
      }
      Method (ECBG, 0, NotSerialized)
      {
        Return (BLAC)
      }
      Method (ECSL, 0, NotSerialized)
      {
        Return (5)
      }
      Method (ECLL, 0, NotSerialized)
      {
        Return (100)
      }
      Method (GSWS, 0, NotSerialized)
      {
        If (ELID)
        {
          Store (Zero, ELID) /* \_SB_.PCI0.LPC_.EC__.ELID */
            Return (Not (LSTE)) /* \_SB_.PCI0.LPC_.EC__.LSTS */
        }

        Return (Zero)
      }

      Method (_STA, 0, NotSerialized)  // _STA: Status
      {
        Return (0x0F)
      }

      Method (_INI, 0, NotSerialized)  // _INI: Initialize
      {
      }

      Method (MHKV, 0, NotSerialized)
      {
        Return (0x0100)
      }
    }
    Method (_Q20) {  //AC in/out
      Notify(AC, 0x80)
      Notify(\_SB.PCI0.LPC.EC.BAT0,0x80) //Battery Status Changed
      Notify(\_SB.PCI0.LPC.EC.BAT0,0x81) //Battery info   Changed
    }
    Method (_Q22) {  //BAT in/out
      Notify(AC, 0x80)
      Notify(\_SB.PCI0.LPC.EC.BAT0,0x80) //Battery Status Changed
      Notify(\_SB.PCI0.LPC.EC.BAT0,0x81) //Battery info   Changed
    }

    Device (BAT0)
    {
      Name (_HID, EISAID("PNP0C0A"))  // _HID: Hardware ID
      Name (_UID, 0x00)  // _UID: Unique ID
      Method (_STA, 0, NotSerialized)  // _STA: Status
      {
        if (AST1){
          Return (0x1F)  // Device present,enabled
        }
        else{
          Return (0x00) // Device not present
        }

      }
      Method(_BIF, 0) {
        Name (BIFP, Package()
        {
          0x01,          //Power Unit    //0,mW; 1,mA
          0xFFFFFFFF,    //Design Capacity
          0xFFFFFFFF,    //Last Full Charge Capacity
          0x01,          //Battery Technology, 1:rechargeable, 0:non-rechargeable
          0xFFFFFFFF,    //Design Voltage
          0xFFFFFFFF,    //Design Capacity of Warning 10%
          0xFFFFFFFF,    //Design Capacity of Low 5%
          0xFFFFFFFF,    //Battery Capacity Granularity 1 1%
          0xFFFFFFFF,    //Battery Capacity Granularity 2
          "LoongsonM",   //Model Number
          "LoongsonS",   //Serial Number
          "LoongsonB",   //Battery Type
          "OEML"         //OEM Information
        })

        Multiply(BDVH, 0x100, local1) // Voltage Design Capacity
        Add(BDVL, local1, local1)
        Store(local1,Index(BIFP,4))

        Multiply(BDCH, 0x100, local1) // Battery Design Capacity
        Add(BDCL, local1, local1)
        Store(local1,Index(BIFP,1))

        Divide(local1, 10, local2, Index(BIFP,5))       // Design Capacity of Warning
        Divide(local1, 20, local2, Index(BIFP,6))       // Design Capacity of Low
        Divide(local1,100, local2, Index(BIFP,7))       // Granularity between Low and warning
        Divide(local1,100, local2, Index(BIFP,8))       // Granularity between Warning and full

        Multiply(BFCH, 0x100, local1) // Last Full Charge Capacity
        Add(BFCL, local1, local1)
        Store(local1,Index(BIFP,2))

        Multiply(BSNH, 0x100, local1) //Serial Number
        Add(BSNL, local1, local1)
        Store(local1,Index(BIFP,10))

        Return(BIFP)
      } //END _BIF

      Method ( _BST, 0) {          //present battery status
        Name (BSTP, Package () {
          0xFFFFFFFF,     // Battery state default discharging
          0xFFFFFFFF,     // Battery Present Rate
          0xFFFFFFFF,     // Battery remaining capacity
          0xFFFFFFFF,     // Battery present Voltage
        })              // End of Package

        if (RPWR) {          // AC Attach online
          if (BCG1) {     // Battery setting
            store(2,Index(BSTP,0))      // Battery charging
          }
          else {
            store(0,Index(BSTP,0))      // No Battery/Full
          }
        }
        else {              //AC offline
          store(1,Index(BSTP,0))          // Battery discharging
        }

        Multiply(BCH, 0x100, local1)   // Battery remaining capacity
        Add(BCL, local1, Index(BSTP,1))
        Multiply(BCH, 0x100, local1) // Battery Current
        Add(BCL, local1, Index(BSTP,1))
        Multiply(BRCH, 0x100, local1)   // Battery remaining capacity
        Add(BRCL, local1, Index(BSTP,2))
        Multiply(BVH, 0x100, local1)   // Battery present Voltage
        Add(BVL, local1, Index(BSTP,3))
        Return (BSTP)
      }  //END _BST
    } //BAT0

    Device (AC)
    {
      Name (_HID, "ACPI0003" /* Power Source Device */)  // _HID: Hardware ID
      Name (_PCL, Package (0x01)  // _PCL: Power Consumer List
      {
        \_SB
      })

      Method (_PSR, 0, NotSerialized)  // _PSR: Power Source
      {
        Return (RPWR) //1: /*On-line*/, 0: /*Off-line*/
      }
    }
  } //end of EC
  Device (LID)
  {
    Name (_HID, EisaId ("PNP0C0D") /* Lid Device */)  // _HID: Hardware ID
    Method (_LID, 0, NotSerialized)  // _LID: lid status
    {
      Return(\_SB.PCI0.LPC.EC.LSTE)
    }
  }
}
