#!/bin/sh

# (c) 2007 Miriam Ruiz <little_miry@yahoo.es>
# (c) 2015 Peter Pentchev <roam@ringlet.net>
# 
# This program is free software; you can redistribute it and/or modify
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
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

ARG="$1"
GAME_FILE=""

tempf=`mktemp -t fenix.XXXXXX`
trap "rm -f -- '$tempf'" EXIT HUP INT QUIT TERM

single_file() {
	local dir="$1"

	local count=0
	if [ -d "$dir" ] && [ -r "$dir" ]; then
		find -- "$dir" -mindepth 1 -maxdepth 1 -type f -name '*.prg' > "$tempf"
		count=`wc -l < "$tempf"`
	fi
	if [ "$count" -eq 1 ]; then
		cat -- "$tempf"
	fi
}

if [ -z "$GAME_FILE" ] && [ -z "$ARG" ]; then
	GAME_FILE=`single_file '.'`
fi

if [ -z "$GAME_FILE" ] && [ -d "$ARG" ]; then
	GAME_FILE=`single_file "$ARG"`
	if [ -z "$GAME_FILE" ]; then
		echo "Cannot choose a game file in the directory \"$ARG\"" >&2
		exit 1
	fi
fi

if [ -z "$GAME_FILE" ] && [ -f "$ARG" ]; then
	GAME_FILE="$ARG"
fi

if [ -z "$GAME_FILE" ]; then
	if [ "$#" -eq 1 ]; then
		echo "Game program does not exist in \"$ARG\"" >&2
	else
		echo "Usage: $0" >&2
		echo "Usage: $0 <file.prg>" >&2
		echo "Usage: $0 <directory>" >&2
	fi
	exit 1
fi

if [ ! -f "$GAME_FILE" ]; then
	echo "File \"$GAME_FILE\" does not exist" >&2
	exit 1
fi

rm -f -- "$tempf"
trap '' EXIT HUP INT QUIT TERM

echo "Game File: \"$GAME_FILE\"" >&2
fenix-fxc "$GAME_FILE" -o - | fenix-fxi -t "$GAME_FILE" -
