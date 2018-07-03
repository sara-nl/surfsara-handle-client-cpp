myRule {
       msi_wordcount(*file, *res);
       writeLine("stdout", "*res");
}

INPUT *file="/tempZone/home/rods/libmsi_wordcount.cpp"
OUTPUT ruleExecOut, *name
