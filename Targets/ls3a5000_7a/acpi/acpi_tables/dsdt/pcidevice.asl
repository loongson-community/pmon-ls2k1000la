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

Scope(\_SB.PCI0)
{
  Device (LPC)
  {
    Name (_ADR, 0x00170000)  // _ADR: Address
    Name (_S3D, 0x03)  // _S3D: S3 Device State
    Name (RID, 0x00)
	Device (KBD)
    {
		Method (_HID, 0, NotSerialized)  // _HID: Hardware ID
        {
          Return (0x0303D041)
        }

        Name (_CID, EisaId ("PNP0303") /* IBM Enhanced Keyboard (101/102-key, PS/2 Mouse) */)  // _CID: Compatible ID
        Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
        {
          IO (Decode16,
            0x0060,             // Range Minimum
            0x0060,             // Range Maximum
            0x01,               // Alignment
            0x01,               // Length
          )
          IO (Decode16,
            0x0064,             // Range Minimum
            0x0064,             // Range Maximum
            0x01,               // Alignment
            0x01,               // Length
          )
          IRQNoFlags (){1}
        })
	}

    Device (MOU)
    {
        Name (_HID, EisaId ("LEN2020"))  // _HID: Hardware ID
        Name (_CID, EisaId ("PNP0F13"))  // _CID: Compatible ID
        Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
        {
          IRQNoFlags ()
          {12}
        })
    }
  } //end of LPC

  /* Host controller (XHCI) */
  Device (PCIB)
  {
    Name (_ADR, 0x000a0000)
    Device (XHCI)
    {
      Name (_ADR, 0x00)
      Name (_S3D, 4)
      Name (_S4D, 4)
      PowerResource (PUBS, 0x03, 0x0000)
      {
        Name (_STA, One)
        Method (_ON, 0, NotSerialized)  // _ON_: Power On
        {
          Store (One, _STA)
        }

        Method (_OFF, 0, NotSerialized)  // _OFF: Power Off
        {
          Store (Zero, _STA)
        }
      }
      Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
      {
        Return (\_SB.PCI0.PCID (Arg0, Arg1, Arg2, Arg3))
      }
      Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
      {
        \_SB.PCI0.PCIB.XHCI.PUBS
      })
      Name (_PR3, Package (0x01)  // _PR1: Power Resources for D1
      {
        \_SB.PCI0.PCIB.XHCI.PUBS
      })
    }  /*end XHCI*/
  }
  /* Host controller (EHCI) */
  Device (USB0) {
    Name (_ADR, 0x00040001)
    Device (RHUB) {
      Name (_ADR, 0x00000000) // must be zero for USB root hub
          // Root hub, port 1
      Device (PRT1) {
        Name (_ADR, 0x00000001)
        // USB port configuration object. This object returns the system
          // specific USB port configuration information for port number 1
          // Must match the _UPC declaration for USB1.RHUB.PRT1 as it is this
          // host controller’s companion
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            // provide physical port location info for port 1
            // Must match the _UPC declaration for USB1.RHUB.PRT1 as it is this
            // host controller’s companion
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
               Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
                     {
                       Buffer (0x14) {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            /*           Revision : 02     */
            /*        IgnoreColor : 01     */
            /*              Color : 000000 */
            /*              Width : 0000   */
            /*             Height : 0000   */
            /*        UserVisible : 01     */
            /*               Dock : 00     */
            /*                Lid : 00     */
            /*              Panel : 00     */
            /*   VerticalPosition : 00     */
            /* HorizontalPosition : 01     */
            /*              Shape : 02     */
            /*   GroupOrientation : 00     */
            /*         GroupToken : 00     */
            /*      GroupPosition : 00     */
            /*                Bay : 00     */
            /*          Ejectable : 00     */
            /*  OspmEjectRequired : 00     */
            /*      CabinetNumber : 00     */
            /*     CardCageNumber : 00     */
            /*          Reference : 00     */
            /*           Rotation : 00     */
            /*              Order : 00     */
            /*     VerticalOffset : 0000   */
            /*   HorizontalOffset : 0000   */
            }
          })
          Return(PLDP)
        }
        }  //Device( PRT1)

      Device (PRT2) {
        Name (_ADR, 0x00000002)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT2)
      Device (PRT3) {
        Name (_ADR, 0x00000003)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0xa9, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT3)
    }  //Device( RHUB)
    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
    {
      \_SB.PCI0.PCIB.XHCI.PUBS
    })
    Method (_PRW, 0, NotSerialized)      // _PRW: Power Resources for Wake
    {
      Return (Package (0x02)
      {
        0xa,
        0x03
      })
    }
  }  //Device( USB0)

  // Companion Host controller (OHCI or UHCI)
  Device (USB1) {
    Name (_ADR, 0x00040000)
    Device (RHUB) {
      Name (_ADR, 0x00000000) // must be zero for USB root hub
      // Root hub, port 1
      Device(PRT1) {
        Name (_ADR, 0x00000001)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14) {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT3)
      Device(PRT2) {
        Name (_ADR, 0x00000002)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      } //Device(PRT3)
      Device(PRT3) {
        Name (_ADR, 0x00000003)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0xa9, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT3)
    }  //Device(RHUB)
  }  //Device(USB1)

  /* Host controller (EHCI) */
  Device (USB2) {
    Name (_ADR, 0x00050001)
    Device (RHUB) {
      Name (_ADR, 0x00000000) // must be zero for USB root hub
          // Root hub, port 1
      Device (PRT1) {
        Name (_ADR, 0x00000001)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14) {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device( PRT1)
      Device (PRT2) {
        Name (_ADR, 0x00000002)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT2)
      Device (PRT3) {
        Name (_ADR, 0x00000003)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0xa9, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT3)
    }  //Device( RHUB)
    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
    {
      \_SB.PCI0.PCIB.XHCI.PUBS
    })
    Method (_PRW, 0, NotSerialized)      // _PRW: Power Resources for Wake
    {
      Return (Package (0x02)
      {
        0xd,
        0x03
      })
    }
  }  //Device( USB2)

  // Companion Host controller (OHCI or UHCI)
  Device (USB3) {
    Name (_ADR, 0x00050000)
    Device (RHUB) {
      Name (_ADR, 0x00000000) // must be zero for USB root hub
      // Root hub, port 1
      Device(PRT1) {
        Name (_ADR, 0x00000001)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14) {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT3)
      Device(PRT2) {
        Name (_ADR, 0x00000002)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0x01, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      } //Device(PRT3)
      Device(PRT3) {
        Name (_ADR, 0x00000003)
        Method(_UPC,0,Serialized)
        {
          Name (UPCP, Package(){
            0xFF,      // Port is connectable
            0x00,      // Connector type – Type ‘A’
            0x00000000,    // Reserved 0 – must be zero
            0x00000000})    // Reserved 1 – must be zero
            Return(UPCP)
        }
        Method(_PLD,0,Serialized)
        {
          Name (PLDP, Package (0x01)  // _PLD: Physical Location of Device
          {
            Buffer (0x14)
            {
            /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0008 */  0xa9, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0010 */  0x00, 0x00, 0x00, 0x00                           /* .... */
            }
          })
          Return(PLDP)
        }
      }  //Device(PRT3)
    }  //Device(RHUB)
  }  //Device(USB3)

  Device (NIC0)
  {
    Name (_ADR, 0x00030000)
    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
    {
      \_SB.PCI0.PCIB.XHCI.PUBS
    })
    Method (_PRW, 0, NotSerialized)      // _PRW: Power Resources for Wake
    {
      Return (Package (0x02)
      {
        0x5,
        0x03
      })
    }
  }
  Device (NIC1)
  {
    Name (_ADR, 0x00030001)
    Name (_PR0, Package (0x01)  // _PR0: Power Resources for D0
    {
      \_SB.PCI0.PCIB.XHCI.PUBS
    })
    Method (_PRW, 0, NotSerialized)      // _PRW: Power Resources for Wake
    {
      Return (Package (0x02)
      {
        0x6,
        0x03
      })
    }
  }
  Name (HDTF, Buffer (0x0E)
  {
    /* 0000 */  0x02, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xEF, 0x00,  /* ........ */
  })

  Name (HPTF, Buffer (0x15)
  {
    /* 0000 */  0x02, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xEF, 0x10,  /* ........ */
    /* 0008 */  0x03, 0x00, 0x00, 0x00, 0xA0, 0xEF, 0x00, 0x00,  /* ........ */
  })

  Device (SAT0)
  {
    Name (_ADR, 0x00080000)
    Device (PORT)
    {
      Name (DIP0, 0x00)
      Name (_ADR, 0xFFFF)  // _ADR: Address
      Method (_SDD, 1, NotSerialized)  // _SDD: Set Device Data
      {
        Store (0x00, DIP0) /* \_SB_.PCI0.SAT0.PORT.DIP0 */
        If (LEqual (SizeOf (Arg0), 0x0200))
        {
          CreateWordField (Arg0, 0x9C, M078)
          If (And (M078, 0x08))
          {
            Store (0x01, DIP0) /* \_SB_.PCI0.SAT0.PORT.DIP0 */
          }
        }
      }

      Method (_GTF, 0, NotSerialized)  // _GTF: Get Task File
      {
        If (DIP0)
        {
          Return (HPTF) /* \_SB_.PCI0.HPTF */
        }
        Return (HDTF) /* \_SB_.PCI0.HDTF */
      }
    }
  }

  Device (SAT1)
  {
    Name (_ADR, 0x00080001)
    Device (PORT)
    {
      Name (DIP1, 0x00)
      Name (_ADR, 0xFFFF)  // _ADR: Address
      Method (_SDD, 1, NotSerialized)  // _SDD: Set Device Data
      {
        Store (0x00, DIP1) /* \_SB_.PCI0.SAT1.PORT.DIP1 */
        If (LEqual (SizeOf (Arg0), 0x0200))
        {
          CreateWordField (Arg0, 0x9C, M078)
          If (And (M078, 0x08))
          {
            Store (0x01, DIP1) /* \_SB_.PCI0.SAT1.PORT.DIP1 */
          }
        }
      }

      Method (_GTF, 0, NotSerialized)  // _GTF: Get Task File
      {
        If (DIP1)
        {
          Return (HPTF) /* \_SB_.PCI0.HPTF */
        }
        Return (HDTF) /* \_SB_.PCI0.HDTF */
      }
    }
  }

  Device (SAT2)
  {
    Name (_ADR, 0x00080002)
    Device (PORT)
    {
      Name (DIP2, 0x00)
      Name (_ADR, 0xFFFF)  // _ADR: Address
      Method (_SDD, 1, NotSerialized)  // _SDD: Set Device Data
      {
        Store (0x00, DIP2) /* \_SB_.PCI0.SAT2.PORT.DIP2 */
        If (LEqual (SizeOf (Arg0), 0x0200))
        {
          CreateWordField (Arg0, 0x9C, M078)
          If (And (M078, 0x08))
          {
            Store (0x01, DIP2) /* \_SB_.PCI0.SAT2.PORT.DIP2 */
          }
        }
      }

      Method (_GTF, 0, NotSerialized)  // _GTF: Get Task File
      {
        If (DIP2)
        {
          Return (HPTF) /* \_SB_.PCI0.HPTF */
        }
        Return (HDTF) /* \_SB_.PCI0.HDTF */
      }
    }
  }
}
