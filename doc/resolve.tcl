#!/bin/sh
#
# FILENAME
#	resolve.tcl
#
# AUTHOR
#	Stefan Sinnige <ssinnige@geocities.com>
#
# DESCRIPTION
#	This Tcl script reads lines from the standard input, executes any 
#	command and writes the output to user-defined output. A command is
#	enclosed by "<@@" and ">" and is only considered as a command if
#	the command sequence appears at the start of the line. If such
#	a command is encountered, it is executed, with the rest of the
#	line as a single parameter. 
#	The command file is specified as parameter of this script. It
#	should contain the commands for each command encountered. If a
#	command is not specified, the parameter contents itself is 
#	ouputed to the standard output.
#
# COPYRIGHT, LICENSE AND DISCLAIMER
#
#	Copyright (C) 1997-2000, Stefan Sinnige.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU Lesser General Public License as 
#	published by the Free Software Foundation; either version 2.1 of the 
#	License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Lesser Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, 
#	USA.
#
#       For the full GNU Lesser General Public License, see the 'LICENSE' file.
#\
exec tclsh "$0" "$@"

# By default, output goes to stdout. The command file can set it to something
# else by setting the 'outfd' to its own channel-id.

set outfd stdout

# If there is a command file, read it

if {$argc == 1} {
    source [lindex $argv 0]
}

# Copy every character and if preceeded by "<@@", treat it as a command

while {[gets stdin line] != -1} {
    if [regexp "<@@(.*)>(.*)" $line all command parameter] {
        if {[info commands $command] == $command} {
            $command $parameter
	} else {
	    puts $outfd $parameter
	}
    } else {
        puts $outfd $line
    }
}

