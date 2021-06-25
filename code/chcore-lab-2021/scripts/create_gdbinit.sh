#!/bin/bash

TEST=$1
cat << EOF > .gdbinit
set architecture aarch64
target remote localhost:1234

define add-symbol-file-auto
	# Parse .text address to temp file
	shell echo set \\\$text_address=\$(readelf -WS \$arg0 | grep .text | awk '{ print "0x"\$5  }') >/tmp/temp_gdb_text_address.txt
	# Source .text address
	source /tmp/temp_gdb_text_address.txt
	#  Clean tempfile
	shell rm -f /tmp/temp_gdb_text_address.txt
	# Load symbol table
	add-symbol-file \$arg0 \$text_address
end

add-symbol-file-auto ./build/kernel.img
add-symbol-file-auto ./user/build/ramdisk/$1.bin
EOF
