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

/*
 * Original Table Header:
 *   Signature  "DSDT"
 *   Length     OP
 *   Revision   0x02
 *   Checksum   0x0
 *   OEM ID     "LOONGS"
 *   OEM Table ID   "LOONGSON"
 *   OEM Revision   0x00000000
 *   Compiler ID    "INTL"
 *   Compiler Version 0x20180629
 */

DefinitionBlock ("Dsdt.aml", "DSDT", 2, "LOONGS", "LOONGSON", 0x00000001)
{
    include ("pcietree.asl")
    include ("pcidevice.asl")
    //include ("ec.asl")
    include ("cpu.asl")
    include ("platform.asl")
    //include ("tz.asl")
    Scope (\_SB.PCI0)
    {
      Name (PCIG, ToUUID ("e5c937d0-3553-4d7a-9117-ea4d19c3434d") /* Device Labeling Interface */)
    Method (PCID, 4, Serialized)
    {
      If (LEqual (Arg0, PCIG))
      {
        If (LGreaterEqual (Arg1, 0x03))
        {
          If (LEqual (Arg2, 0x00))
          {
            Return (Buffer (0x02)
            {
              0x01, 0x03
            })
          }

          If (LEqual (Arg2, 0x09))
          {
            Return (Package (0x05)
            {
              0xC350,
              Ones,
              Ones,
              0xC350,
              Ones
            })
          }
        }
      }

      Return (Buffer (0x01)
      {
        0x00
      })
    }
  }//end \_SB

  Name (\_S0, Package (0x04)  // _S0_: S0 System State
  {
    0x00,
    0x00,
    0x00,
    0x00
  })
  Name (\_S3, Package (0x04)  // _S3_: S3 System State
  {
    0x05,
    0x05,
    0x00,
    0x00
  })
  Name (\_S4, Package (0x04)  // _S4_: S4 System State
  {
    0x06,
    0x06,
    0x00,
    0x00
  })
  Name (\_S5, Package (0x04)  // _S5_: S5 System State
  {
    0x07,
    0x07,
    0x00,
    0x00
  })

  Name (SADR, 0x800000001c000500)

  Scope (\_GPE)
  {
  }
}
