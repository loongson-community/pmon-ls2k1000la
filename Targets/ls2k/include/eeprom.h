void i2c_init();
int ls2k_eeprom_read_cur(unsigned char *buf);
int ls2k_eeprom_read_rand(unsigned short data_addr, unsigned char *buf);
int ls2k_eeprom_read_seq(unsigned short data_addr, unsigned char *buf, int count);
int ls2k_eeprom_write_byte(unsigned short data_addr, unsigned char *buf);
int ls2k_eeprom_write_page(unsigned short data_addr, unsigned char *buf, int count);
int mac_read(unsigned short data_addr, unsigned char *buf, int count);
int mac_write(unsigned short data_addr, unsigned char *buf, int count);

