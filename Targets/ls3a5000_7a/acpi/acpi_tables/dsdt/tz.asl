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

Scope (\_TZ)
{
  ThermalZone (THM0)
  {
    Name(_TZP, 300)   //polling delay

    Method (_TMP, 0, NotSerialized)  // _TMP: Temperature
    {
        Store (\_SB.PCI0.LPC.EC.CTMP, Local0)
        Return (C2K (Local0))
    }

    Method (_CRT, 0, NotSerialized)  // _CRT: Critical Temperature
    {
        Return (C2K (0x60))
    }

    Method (_PSL, 0, Serialized)  // _PSL: Passive List
    {
        Return (Package (0x04)
        {
            \_SB.C000,
            \_SB.C001,
            \_SB.C002,
            \_SB.C003
        })
    }

    Method (_PSV, 0, NotSerialized)  // _PSV: Passive Temperature
    {
        Return (C2K (0x46))
    }

    Name (_TC1, Zero)  // _TC1: Thermal Constant 1
    Name (_TC2, 0x32)  // _TC2: Thermal Constant 2
    Name (_TSP, 300)   // _TSP: Thermal Sampling Period
  }

  Method (C2K, 1, NotSerialized)   //Celsius to Kelvin
  {
    Add (Multiply (Arg0, 0x0A), 0x0AAC, Local0)
    If (LLessEqual (Local0, 0x0AAC))
    {
        Store (0x0BB8, Local0)
    }

    If (LGreater (Local0, 0x0FAC))
    {
        Store (0x0BB8, Local0)
    }

    Return (Local0)
  }
}
