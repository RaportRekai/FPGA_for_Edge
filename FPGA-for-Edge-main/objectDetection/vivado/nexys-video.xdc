# Pin Assigment for Nexys Video Board
set_property PACKAGE_PIN R4 [get_ports clock]
set_property PACKAGE_PIN W21 [get_ports oled_spi_clk]
set_property PACKAGE_PIN E22 [get_ports {start[0]}]
set_property PACKAGE_PIN B22 [get_ports reset]
set_property PACKAGE_PIN U21 [get_ports oled_reset]
set_property PACKAGE_PIN V22 [get_ports oled_vdd]
set_property PACKAGE_PIN W22 [get_ports oled_dc_n]
set_property PACKAGE_PIN P20 [get_ports oled_vbat]
set_property PACKAGE_PIN Y22 [get_ports oled_spi_data]
set_property IOSTANDARD LVCMOS18 [get_ports clock]
set_property IOSTANDARD LVCMOS18 [get_ports oled_spi_clk]
set_property IOSTANDARD LVCMOS18 [get_ports {start[0]}]
set_property IOSTANDARD LVCMOS18 [get_ports reset]
set_property IOSTANDARD LVCMOS18 [get_ports oled_dc_n]
set_property IOSTANDARD LVCMOS18 [get_ports oled_reset]
set_property IOSTANDARD LVCMOS18 [get_ports oled_spi_data]
set_property IOSTANDARD LVCMOS18 [get_ports oled_vbat]
set_property IOSTANDARD LVCMOS18 [get_ports oled_vdd]
set_property IOSTANDARD LVCMOS18 [get_ports tx]
set_property IOSTANDARD LVCMOS18 [get_ports txComplete]
set_property PACKAGE_PIN AA19 [get_ports tx]
set_property PACKAGE_PIN T14 [get_ports txComplete]