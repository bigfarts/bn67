// Native SP Navi records store a sentinel above 1000. The power resolver uses
// its offset to select a time-dependent value from the SP damage table. Store
// the first (maximum) value from each row directly so these chips no longer
// scale with the corresponding Navi's deletion time.
.definelabel navi_chip_table, 0x08021DA8

.macro fix_sp_power,chip_id,power
    .org navi_chip_table + chip_id * 0x2C + 0x1A
    .dh power
.endmacro

fix_sp_power 0x0E8,210 // ElecMan3
fix_sp_power 0x0EB,220 // SlashMn3
fix_sp_power 0x0EE,210 // EraseMn3
fix_sp_power 0x0F4,120 // SpoutMn3
fix_sp_power 0x0F7,280 // TmhkMan3
fix_sp_power 0x0FA,160 // TenguMn3
fix_sp_power 0x0FD,130 // GrndMan3
fix_sp_power 0x100,200 // DustMan3
fix_sp_power 0x103,250 // BlastMn3
fix_sp_power 0x106,270 // DiveMan3
fix_sp_power 0x10C,190 // JudgeMn3
fix_sp_power 0x10F,240 // ElmntMn3
