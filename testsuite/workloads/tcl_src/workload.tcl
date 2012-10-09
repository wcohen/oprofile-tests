# workload.tcl
#  Copyright (C) 2012 IBM

# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

if { $argc != 1 } {
    set count 1000
} else {
    set count [lindex $argv 0]
}

puts $count

set j 23523
set k 15431
set m 73141
set n 93802
for {set i 0} {$i < $count} {incr i} {
    set j [expr {($k * $m - ($n >> 2)) % 4294967296}]
    set k [expr {($m + $n * ($j ^ $k)) % 4294967296}]
    set m [expr {($n * $j - ($k << 2)) % 4294967296}]
    set n [expr {($j + $k * ($m ^ $k)) % 4294967296}]
}
exec dd bs=16 if=/dev/urandom of=/dev/null count=$count
