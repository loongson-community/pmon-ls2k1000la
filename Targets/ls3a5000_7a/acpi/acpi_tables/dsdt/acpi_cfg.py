#/bin/python2
# Use python2 to avoid no-python3 environment

import os
import re

ROOT_PATH = os.getcwd() # in zloader now!

##############################################################
#             Deal with some defination in Pmon              #
##############################################################
IDENT_FILE=open(ROOT_PATH + "/../Targets/ls3a5000_7a/compile/loongson/Makefile", "r")
IDENT = IDENT_FILE.readline()

TOT_NODE_NUM = int(re.sub("0x", "", re.search("(?<=-DTOT_NODE_NUM)\S*", IDENT).group().strip("=").strip("\"")), 16)
TOT_7A_NUM = int(re.search("(?<=-DTOT_7A_NUM)\S*", IDENT).group().strip("=").strip("\""))
SOUTH_BRIDGE = ["LS7A2000" if int(1 if re.search("(?<=-DLS7A2000)\S*", IDENT) else 0) else ""]

IDENT_FILE.close()

if TOT_NODE_NUM <= 4:
        other_7a_id = 0x1
elif (TOT_NODE_NUM == 8) or (TOT_NODE_NUM == 16):
        other_7a_id = 0x5
else:
        other_7a_id = 0xa


##############################################################
#                some var for pcietree.asl                   #
##############################################################
CommitFlag = 0
Ls7a0Flag = 0
Ls7a1Flag = 0
Flag=0
TGTACPI = []
PCI0=[]
PCI1=[]
DEFLAG = True

DST=open(ROOT_PATH + "/../Targets/ls3a5000_7a/acpi/acpi_tables/dsdt/pcietree.template", "r")
for line in DST.readlines():
    if line.strip() == None:
        continue
    # deal Certificates Describe
    if line.strip().startswith("/**"):
        CommitFlag += 1
    if line.strip().endswith("**/"):
        CommitFlag -= 1
    if CommitFlag or line.strip().startswith("//"):
        TGTACPI.append(line)
        continue

    if line.strip().startswith("#ifdef"):
        if line.strip().split(" ")[1] in SOUTH_BRIDGE:
            continue
        else:
            DEFLAG = False
            continue
    elif line.strip().startswith("#else"):
        DEFLAG = not(DEFLAG)
        continue
    elif line.strip().startswith("#endif"):
        DEFLAG = True
        continue
    if not(DEFLAG):
        continue

    # count '{' and '}' if find Device PCI0
    if re.search(".*Device.*PCI0.*{*", line):
        Ls7a0Flag += 1
        if re.search("(?<!//).*{.*", line) and re.search("(?<!/*).*{.*", line):
            Flag += 1
        if re.search("(?<!//).*}.*", line) and re.search("(?<!/*).*}.*", line):
            Flag -= 1
        PCI0.append(line)
        continue
    if Ls7a0Flag:
        if re.search("(?<!//).*{", line) and re.search("(?<!/\\*).*{.*", line):
            Flag += 1
        if re.search("(?<!//).*}", line) and re.search("(?<!/\\*).*}.*", line):
            Flag -= 1
        PCI0.append(line)
        if Flag == 0:
            Ls7a0Flag -= 1
        continue

    # count '{' and '}' if find Device PCI1
    if re.search(".*Device.*PCI1.*{*", line):
        Ls7a1Flag += 1
        if re.search("(?<!//).*{.*", line) and re.search("(?<!/*).*{.*", line):
            Flag += 1
        if re.search("(?<!//).*}.*", line) and re.search("(?<!/*).*}.*", line):
            Flag -= 1
        PCI1.append(line)
        continue
    if Ls7a1Flag:
        if re.search("0x\w{12}", line):
            line = re.sub("(?<=0x)\w(?=\w{11}\s*,)", str(other_7a_id), line)
        if re.search("(?<!//).*{", line) and re.search("(?<!/\\*).*{.*", line):
            Flag += 1
        if re.search("(?<!//).*}", line) and re.search("(?<!/\\*).*}.*", line):
            Flag -= 1
        PCI1.append(line)
        if Flag == 0:
            Ls7a1Flag -= 1
        continue

    TGTACPI.append(line)

DST.close()

##############################################################
#                  write to pcietree.asl                     #
##############################################################
DST=open(ROOT_PATH + "/../Targets/ls3a5000_7a/acpi/acpi_tables/dsdt/pcietree.asl", "w")

if TOT_7A_NUM == 2:
    TGTACPI = TGTACPI[:-1] + PCI0 + PCI1 + TGTACPI[-1:]
else:
    TGTACPI = TGTACPI[:-1] + PCI0 + TGTACPI[-1:]

for item in TGTACPI:
    DST.write(item)

DST.close()


##############################################################
#                     Deal with Cpu.asl                      #
##############################################################
CommitFlag = 0
# '{' is begin number set 1 but not 0 because it deal completed parentheses
Flag=0
COMMENTS=[]
TGTACPI = []
CORENUM = TOT_NODE_NUM * 4
coreid = 0

DST=open(ROOT_PATH + "/../Targets/ls3a5000_7a/acpi/acpi_tables/dsdt/cpu.template", "r")

for line in DST.readlines():
    if line.strip() == None:
        continue
    if re.search("(?<=Device).*C\w{3}", line):
        Flag = 1
    if Flag:
        TGTACPI.append(line)
    else:
        COMMENTS.append(line)

DST.close()

DST=open(ROOT_PATH + "/../Targets/ls3a5000_7a/acpi/acpi_tables/dsdt/cpu.asl", "w")

for item in COMMENTS:
    DST.write(item)

for i in range(CORENUM):
    for item in TGTACPI[:-1]:
        if re.search("(?<=Device).*C\w{3}", item):
            new_item = re.sub(r"C\w{3}", "{}{:0>3X}".format("C", i), item)
            DST.write(new_item)
            continue
        if re.search("(?<=Name \(_UID, )\w+", item):
            new_item = re.sub("(?<=Name \(_UID, )\w+", "0x{:X}".format(i+1), item)
            DST.write(new_item)
            continue
        DST.write(item)
DST.write(TGTACPI[-1])

DST.close()

print("deal acpi asl file done!")
