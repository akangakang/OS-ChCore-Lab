#!/bin/bash
cd ./tests/mm
cd buddy
cmake ./
make
./test_buddy > buddy.out
make clean
cd ../
cd page_table
cmake ./
make
./test_aarch64_page_table > page_table.out
make clean
cd ../../../

